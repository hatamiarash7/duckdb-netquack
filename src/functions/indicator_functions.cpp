// Copyright 2026 Arash Hatami

#include "indicator_functions.hpp"

#include <cctype>

#include "../utils/tld_lookup.hpp"
#include "ip_functions.hpp"
#include "validation_functions.hpp"

namespace duckdb {

using Extractor = std::vector<std::string> (*)(const std::string_view &);

static void ExtractToList(DataChunk &args, Vector &result, Extractor extract) {
	const auto &input_vector = args.data[0];
	auto list_data = FlatVector::GetData<list_entry_t>(result);
	auto &result_validity = FlatVector::Validity(result);
	idx_t offset = ListVector::GetListSize(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		auto matches = extract(input);

		// Reserve may reallocate the child buffer, so fetch its data afterwards
		ListVector::Reserve(result, offset + matches.size());
		auto &child = ListVector::GetEntry(result);
		auto child_data = FlatVector::GetData<string_t>(child);
		for (idx_t k = 0; k < matches.size(); k++) {
			child_data[offset + k] = StringVector::AddString(child, matches[k]);
		}

		list_data[i].offset = offset;
		list_data[i].length = matches.size();
		offset += matches.size();
	}

	ListVector::SetListSize(result, offset);
}

void ExtractURLsFunction(DataChunk &args, ExpressionState &, Vector &result) {
	ExtractToList(args, result, netquack::ExtractURLs);
}

void ExtractDomainsFunction(DataChunk &args, ExpressionState &, Vector &result) {
	ExtractToList(args, result, netquack::ExtractDomains);
}

void ExtractIPsFunction(DataChunk &args, ExpressionState &, Vector &result) {
	ExtractToList(args, result, netquack::ExtractIPs);
}

namespace netquack {

static bool IsAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool IsDigit(char c) {
	return c >= '0' && c <= '9';
}

static bool IsAlnum(char c) {
	return IsAlpha(c) || IsDigit(c);
}

static bool IsHex(char c) {
	return IsDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Characters that glue an indicator to a surrounding word (letters, digits, '_', and non-ASCII bytes)
static bool IsWordByte(char c) {
	return IsAlnum(c) || c == '_' || static_cast<unsigned char>(c) >= 0x80;
}

// ---------------------------------------------------------------------------
// URLs
// ---------------------------------------------------------------------------

static bool IsSchemeChar(char c) {
	return IsAlnum(c) || c == '+' || c == '-' || c == '.';
}

// A URL runs until whitespace, a control character, or a quote / angle bracket delimiter
static bool IsURLChar(char c) {
	auto u = static_cast<unsigned char>(c);
	return u > 0x20 && u != 0x7f && c != '"' && c != '\'' && c != '<' && c != '>' && c != '`';
}

// Drops trailing sentence punctuation and closing brackets that have no opening partner inside the URL
static size_t TrimURLEnd(const std::string_view &input, size_t start, size_t end) {
	int parens = 0, squares = 0, braces = 0;
	for (size_t k = start; k < end; k++) {
		char c = input[k];
		parens += (c == '(') - (c == ')');
		squares += (c == '[') - (c == ']');
		braces += (c == '{') - (c == '}');
	}

	while (end > start) {
		char c = input[end - 1];
		if (c == '.' || c == ',' || c == ';' || c == ':' || c == '!' || c == '?') {
			end--;
		} else if (c == ')' && parens < 0) {
			parens++;
			end--;
		} else if (c == ']' && squares < 0) {
			squares++;
			end--;
		} else if (c == '}' && braces < 0) {
			braces++;
			end--;
		} else {
			break;
		}
	}
	return end;
}

std::vector<std::string> ExtractURLs(const std::string_view &input) {
	std::vector<std::string> urls;
	const size_t n = input.size();

	for (size_t i = 0; i < n; i++) {
		if (!IsAlpha(input[i]) || (i > 0 && (IsSchemeChar(input[i - 1]) || IsWordByte(input[i - 1])))) {
			continue;
		}

		size_t scheme_end = i;
		while (scheme_end < n && IsSchemeChar(input[scheme_end])) {
			scheme_end++;
		}
		if (input.substr(scheme_end, 3) != "://") {
			i = scheme_end - 1;
			continue;
		}

		size_t body_start = scheme_end + 3;
		size_t end = body_start;
		while (end < n && IsURLChar(input[end])) {
			end++;
		}
		end = TrimURLEnd(input, i, end);
		if (end > body_start) {
			urls.emplace_back(input.substr(i, end - i));
			i = end - 1;
		} else {
			i = scheme_end + 2;
		}
	}

	return urls;
}

// ---------------------------------------------------------------------------
// Domains
// ---------------------------------------------------------------------------

static bool IsDomainChar(char c) {
	return IsAlnum(c) || c == '-' || c == '.';
}

std::vector<std::string> ExtractDomains(const std::string_view &input) {
	std::vector<std::string> domains;
	const size_t n = input.size();

	size_t i = 0;
	while (i < n) {
		if (!IsDomainChar(input[i])) {
			i++;
			continue;
		}

		size_t start = i;
		while (i < n && IsDomainChar(input[i])) {
			i++;
		}
		size_t end = i;

		// Skip tokens glued to other word characters and email local parts (john.dev@...)
		if ((start > 0 && IsWordByte(input[start - 1])) || (end < n && (IsWordByte(input[end]) || input[end] == '@'))) {
			continue;
		}

		while (start < end && (input[start] == '.' || input[start] == '-')) {
			start++;
		}
		while (end > start && (input[end - 1] == '.' || input[end - 1] == '-')) {
			end--;
		}

		auto token = input.substr(start, end - start);
		auto last_dot = token.rfind('.');
		if (last_dot == std::string_view::npos || !IsValidDomain(token)) {
			continue;
		}

		std::string domain(token);
		for (auto &c : domain) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
		if (isKnownTLD(domain.substr(last_dot + 1))) {
			domains.push_back(std::move(domain));
		}
	}

	return domains;
}

// ---------------------------------------------------------------------------
// IP addresses
// ---------------------------------------------------------------------------

static bool IsIPv6Char(char c) {
	return IsHex(c) || c == ':' || c == '.';
}

// An IPv6 candidate may follow a separator colon ("addr:fe80::1") but not sit inside another address run
static bool IsIPv6Start(const std::string_view &input, size_t pos) {
	if (pos == 0) {
		return true;
	}
	char prev = input[pos - 1];
	if (prev == ':') {
		return pos < 2 || !IsIPv6Char(input[pos - 2]);
	}
	return !IsWordByte(prev) && prev != '.';
}

// Length of a valid IPv6 address at `pos`, retrying without trailing ':' / '.' punctuation, otherwise 0
static size_t MatchIPv6(const std::string_view &input, size_t pos) {
	size_t end = pos;
	size_t colons = 0;
	bool has_hex = false;
	while (end < input.size() && IsIPv6Char(input[end])) {
		colons += input[end] == ':';
		has_hex |= IsHex(input[end]);
		end++;
	}
	if (colons < 2 || !has_hex || (end < input.size() && IsWordByte(input[end]))) {
		return 0;
	}

	while (end > pos + 1) {
		if (IsValidIPv6(std::string(input.substr(pos, end - pos)))) {
			return end - pos;
		}
		char last = input[end - 1];
		if (last != ':' && last != '.') {
			break;
		}
		end--;
	}
	return 0;
}

// Length of a valid IPv4 address at `pos` (ignoring a trailing sentence dot), otherwise 0
static size_t MatchIPv4(const std::string_view &input, size_t pos) {
	if (pos > 0 && (IsWordByte(input[pos - 1]) || input[pos - 1] == '.')) {
		return 0;
	}
	size_t end = pos;
	while (end < input.size() && (IsDigit(input[end]) || input[end] == '.')) {
		end++;
	}
	if (end < input.size() && IsWordByte(input[end])) {
		return 0;
	}
	while (end > pos && input[end - 1] == '.') {
		end--;
	}
	return IsValidIPv4(std::string(input.substr(pos, end - pos))) ? end - pos : 0;
}

std::vector<std::string> ExtractIPs(const std::string_view &input) {
	std::vector<std::string> ips;
	const size_t n = input.size();

	size_t i = 0;
	while (i < n) {
		size_t len = 0;
		if ((IsHex(input[i]) || input[i] == ':') && IsIPv6Start(input, i)) {
			len = MatchIPv6(input, i);
		}
		if (len == 0 && IsDigit(input[i])) {
			len = MatchIPv4(input, i);
		}

		if (len > 0) {
			ips.emplace_back(input.substr(i, len));
			i += len;
		} else {
			i++;
		}
	}

	return ips;
}

} // namespace netquack
} // namespace duckdb
