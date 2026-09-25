// Copyright 2026 Arash Hatami

#include "surt_functions.hpp"

#include "ip_functions.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace duckdb {

template <std::string (*Transform)(const std::string_view &)>
static void ApplyStringTransform(DataChunk &args, Vector &result) {
	const auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		result_data[i] = StringVector::AddString(result, Transform(input));
	}
}

void UrlToSurtFunction(DataChunk &args, ExpressionState &, Vector &result) {
	ApplyStringTransform<netquack::UrlToSurt>(args, result);
}

void SurtToUrlFunction(DataChunk &args, ExpressionState &, Vector &result) {
	ApplyStringTransform<netquack::SurtToUrl>(args, result);
}

namespace netquack {

struct UrlParts {
	std::string scheme;
	std::string host;
	std::string port;
	std::string_view rest; // path, query, and fragment
};

static void ToLowerInPlace(std::string &s) {
	std::transform(s.begin(), s.end(), s.begin(),
	               [](char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; });
}

static bool IsSpace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static std::string_view Trim(std::string_view s) {
	while (!s.empty() && IsSpace(s.front())) {
		s.remove_prefix(1);
	}
	while (!s.empty() && IsSpace(s.back())) {
		s.remove_suffix(1);
	}
	return s;
}

static bool IsSchemeChar(char c, bool first) {
	bool alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	return first ? alpha : (alpha || (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.');
}

// Splits `input` into lowercase scheme, host, and port; userinfo is dropped
static UrlParts SplitUrl(std::string_view input) {
	UrlParts parts;

	size_t sep = input.find("://");
	if (sep != std::string_view::npos && sep > 0) {
		bool valid = true;
		for (size_t k = 0; k < sep && valid; k++) {
			valid = IsSchemeChar(input[k], k == 0);
		}
		if (valid) {
			parts.scheme = std::string(input.substr(0, sep));
			ToLowerInPlace(parts.scheme);
			input.remove_prefix(sep + 3);
		}
	}
	if (parts.scheme.empty() && input.size() >= 2 && input[0] == '/' && input[1] == '/') {
		input.remove_prefix(2);
	}

	size_t auth_end = input.find_first_of("/?#");
	if (auth_end == std::string_view::npos) {
		auth_end = input.size();
	}
	auto authority = input.substr(0, auth_end);
	parts.rest = input.substr(auth_end);

	size_t at = authority.rfind('@');
	if (at != std::string_view::npos) {
		authority.remove_prefix(at + 1);
	}

	size_t port_colon = std::string_view::npos;
	if (!authority.empty() && authority[0] == '[') {
		size_t close = authority.find(']');
		if (close != std::string_view::npos && close + 1 < authority.size() && authority[close + 1] == ':') {
			port_colon = close + 1;
		}
	} else if (std::count(authority.begin(), authority.end(), ':') == 1) {
		port_colon = authority.find(':');
	}
	if (port_colon != std::string_view::npos) {
		parts.port = std::string(authority.substr(port_colon + 1));
		authority = authority.substr(0, port_colon);
	}

	parts.host = std::string(authority);
	ToLowerInPlace(parts.host);
	while (!parts.host.empty() && parts.host.back() == '.') {
		parts.host.pop_back();
	}
	return parts;
}

static bool IsIPHost(const std::string &host) {
	return (!host.empty() && host[0] == '[') || IsValidIPv4(host) || IsValidIPv6(host);
}

static std::string ReverseLabels(std::string_view host, char from, char to) {
	std::string out;
	out.reserve(host.size());
	size_t end = host.size();
	size_t sep;
	while (end > 0 && (sep = host.rfind(from, end - 1)) != std::string_view::npos) {
		out.append(host.substr(sep + 1, end - sep - 1));
		out += to;
		end = sep;
	}
	out.append(host.substr(0, end));
	return out;
}

// Strips a leading "www." or "www<digits>." label, as the Internet Archive SURT canonicalizer does,
// unless only a single label would remain (www.com)
static void StripWww(std::string &host) {
	if (host.compare(0, 3, "www") != 0) {
		return;
	}
	size_t k = 3;
	while (k < host.size() && host[k] >= '0' && host[k] <= '9') {
		k++;
	}
	if (k < host.size() && host[k] == '.' && host.find('.', k + 1) != std::string::npos) {
		host.erase(0, k + 1);
	}
}

static bool IsDefaultPort(const std::string &scheme, const std::string &port) {
	return port.empty() || ((scheme.empty() || scheme == "http") && port == "80") ||
	       (scheme == "https" && port == "443") || (scheme == "ftp" && port == "21");
}

static std::string SortQuery(std::string_view query) {
	std::vector<std::string_view> params;
	while (!query.empty()) {
		size_t amp = query.find('&');
		auto param = query.substr(0, amp);
		if (!param.empty()) {
			params.push_back(param);
		}
		if (amp == std::string_view::npos) {
			break;
		}
		query.remove_prefix(amp + 1);
	}
	std::sort(params.begin(), params.end());

	std::string out;
	for (size_t k = 0; k < params.size(); k++) {
		if (k > 0) {
			out += '&';
		}
		out += params[k];
	}
	return out;
}

std::string UrlToSurt(const std::string_view &input) {
	auto trimmed = Trim(input);
	auto parts = SplitUrl(trimmed);
	if (parts.host.empty()) {
		return std::string(trimmed);
	}

	std::string result;
	if (IsIPHost(parts.host)) {
		result = parts.host;
	} else {
		StripWww(parts.host);
		result = ReverseLabels(parts.host, '.', ',');
	}
	if (!IsDefaultPort(parts.scheme, parts.port)) {
		result += ':';
		result += parts.port;
	}
	result += ')';

	auto rest = parts.rest.substr(0, parts.rest.find('#'));
	size_t qmark = rest.find('?');
	std::string path(rest.substr(0, qmark));
	ToLowerInPlace(path);
	while (path.size() > 1 && path.back() == '/') {
		path.pop_back();
	}
	result += path.empty() ? "/" : path;

	if (qmark != std::string_view::npos) {
		auto query = SortQuery(rest.substr(qmark + 1));
		ToLowerInPlace(query);
		if (!query.empty()) {
			result += '?';
			result += query;
		}
	}

	return result;
}

// The scheme is not part of a SURT key, so http is assumed unless the Heritrix form
// (https://(com,example,)/path) carries one
std::string SurtToUrl(const std::string_view &input) {
	auto surt = Trim(input);
	std::string scheme = "http";

	size_t sep = surt.find("://(");
	if (sep != std::string_view::npos && sep > 0) {
		bool valid = true;
		for (size_t k = 0; k < sep && valid; k++) {
			valid = IsSchemeChar(surt[k], k == 0);
		}
		if (valid) {
			scheme = std::string(surt.substr(0, sep));
			ToLowerInPlace(scheme);
			surt.remove_prefix(sep + 4);
		}
	}

	size_t close = surt.find(')');
	if (close == std::string_view::npos || close == 0) {
		return std::string(Trim(input));
	}
	auto authority = surt.substr(0, close);
	auto rest = surt.substr(close + 1);

	std::string_view port;
	size_t port_colon = std::string_view::npos;
	if (authority[0] == '[') {
		size_t bracket = authority.find(']');
		if (bracket != std::string_view::npos && bracket + 1 < authority.size() && authority[bracket + 1] == ':') {
			port_colon = bracket + 1;
		}
	} else {
		port_colon = authority.find(':');
	}
	if (port_colon != std::string_view::npos) {
		port = authority.substr(port_colon + 1);
		authority = authority.substr(0, port_colon);
	}
	while (!authority.empty() && authority.back() == ',') {
		authority.remove_suffix(1);
	}
	if (authority.empty()) {
		return std::string(Trim(input));
	}

	std::string result = scheme + "://" + ReverseLabels(authority, ',', '.');
	if (!port.empty()) {
		result += ':';
		result += port;
	}
	if (rest.empty() || rest[0] != '/') {
		result += '/';
	}
	result += rest;
	return result;
}

} // namespace netquack
} // namespace duckdb
