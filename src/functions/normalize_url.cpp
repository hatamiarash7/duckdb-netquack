// Copyright 2025 Arash Hatami

#include "normalize_url.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace duckdb {

void NormalizeURLFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		try {
			auto normalized = netquack::NormalizeURL(input);
			result_data[i] = StringVector::AddString(result, normalized);
		} catch (const std::exception &) {
			// On any parsing failure, return original input
			result_data[i] = StringVector::AddString(result, input);
		}
	}
}

namespace netquack {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Lowercase a string in-place
static void ToLower(std::string &s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

// Check if a character is unreserved per RFC 3986
static bool IsUnreservedChar(char c) {
	return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '.' || c == '_' || c == '~';
}

// Convert a hex character to its integer value
static int HexVal(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return -1;
}

// Decode percent-encoded characters that don't need encoding (RFC 3986 normalization).
// Re-encode any that DO need encoding with uppercase hex digits.
static std::string NormalizePercentEncoding(const std::string &s) {
	std::string out;
	out.reserve(s.size());

	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '%' && i + 2 < s.size()) {
			int hi = HexVal(s[i + 1]);
			int lo = HexVal(s[i + 2]);
			if (hi >= 0 && lo >= 0) {
				char decoded = static_cast<char>((hi << 4) | lo);
				if (IsUnreservedChar(decoded)) {
					// Unreserved characters should be decoded
					out += decoded;
				} else {
					// Keep encoded but with uppercase hex
					out += '%';
					out += "0123456789ABCDEF"[(hi & 0xF)];
					out += "0123456789ABCDEF"[(lo & 0xF)];
				}
				i += 2;
				continue;
			}
		}
		out += s[i];
	}

	return out;
}

// Remove dot segments from a path per RFC 3986 section 5.2.4
static std::string RemoveDotSegments(const std::string &path) {
	if (path.empty()) {
		return path;
	}

	std::string input = path;
	std::string output;

	while (!input.empty()) {
		// A: If the input buffer begins with a prefix of "../" or "./"
		if (input.substr(0, 3) == "../") {
			input = input.substr(3);
		} else if (input.substr(0, 2) == "./") {
			input = input.substr(2);
		}
		// B: If the input buffer begins with a prefix of "/./" or "/."
		else if (input.substr(0, 3) == "/./") {
			input = "/" + input.substr(3);
		} else if (input == "/.") {
			input = "/";
		}
		// C: If the input buffer begins with a prefix of "/../" or "/.."
		else if (input.substr(0, 4) == "/../") {
			input = "/" + input.substr(4);
			size_t last_slash = output.rfind('/');
			if (last_slash != std::string::npos) {
				output = output.substr(0, last_slash);
			} else {
				output.clear();
			}
		} else if (input == "/..") {
			input = "/";
			size_t last_slash = output.rfind('/');
			if (last_slash != std::string::npos) {
				output = output.substr(0, last_slash);
			} else {
				output.clear();
			}
		}
		// D: if the input buffer consists only of "." or ".."
		else if (input == "." || input == "..") {
			input.clear();
		}
		// E: move the first path segment (including initial "/" if any) to output
		else {
			size_t seg_start = 0;
			if (input[0] == '/') {
				seg_start = 1;
			}
			size_t next_slash = input.find('/', seg_start);
			if (next_slash == std::string::npos) {
				output += input;
				input.clear();
			} else {
				output += input.substr(0, next_slash);
				input = input.substr(next_slash);
			}
		}
	}

	return output;
}

// Split a query string into sorted key=value pairs
static std::string SortQueryParams(const std::string &query) {
	if (query.empty()) {
		return "";
	}

	std::vector<std::string> params;
	std::istringstream stream(query);
	std::string param;

	while (std::getline(stream, param, '&')) {
		if (!param.empty()) {
			params.push_back(param);
		}
	}

	std::sort(params.begin(), params.end());

	std::string result;
	for (size_t i = 0; i < params.size(); ++i) {
		if (i > 0) {
			result += '&';
		}
		result += params[i];
	}

	return result;
}

// Check if the given port is the default for the scheme
static bool IsDefaultPort(const std::string &scheme, const std::string &port) {
	if (port.empty()) {
		return true;
	}
	if (scheme == "http" && port == "80") {
		return true;
	}
	if (scheme == "https" && port == "443") {
		return true;
	}
	if (scheme == "ftp" && port == "21") {
		return true;
	}
	return false;
}

// Ensure the path has a leading "/" for authority-based URLs
static std::string EnsureLeadingSlash(const std::string &path) {
	if (path.empty() || path[0] != '/') {
		return "/" + path;
	}
	return path;
}

// Remove trailing slashes from the path
static std::string RemoveTrailingSlashes(const std::string &path) {
	if (path.empty()) {
		return "";
	}
	size_t end = path.size();
	while (end > 0 && path[end - 1] == '/') {
		--end;
	}
	return path.substr(0, end);
}

// ---------------------------------------------------------------------------
// Main normalization
// ---------------------------------------------------------------------------

std::string NormalizeURL(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	// Trim whitespace
	size_t start = 0;
	size_t end = input.size();
	while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) {
		++start;
	}
	while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
		--end;
	}
	std::string url = input.substr(start, end - start);

	if (url.empty()) {
		return "";
	}

	// -----------------------------------------------------------------------
	// 1. Parse components
	// -----------------------------------------------------------------------

	std::string scheme;
	std::string authority; // user:pass@host:port
	std::string path;
	std::string query;
	// fragment is intentionally dropped (not meaningful for server)

	size_t pos = 0;

	// Extract scheme
	size_t scheme_end = url.find("://");
	if (scheme_end != std::string::npos && scheme_end < 16) {
		// Validate scheme characters
		bool valid_scheme = true;
		for (size_t k = 0; k < scheme_end; ++k) {
			char c = url[k];
			if (!(std::isalpha(static_cast<unsigned char>(c)) ||
			      (k > 0 && (std::isdigit(static_cast<unsigned char>(c)) || c == '+' || c == '-' || c == '.')))) {
				valid_scheme = false;
				break;
			}
		}
		if (valid_scheme && scheme_end > 0) {
			scheme = url.substr(0, scheme_end);
			pos = scheme_end + 3; // skip "://"
		}
	}

	if (scheme.empty()) {
		// No scheme found - return the input with minimal normalization
		// (just trim whitespace, which we already did)
		return url;
	}

	// Extract authority (everything up to the next / ? or #)
	size_t auth_end = pos;
	while (auth_end < url.size() && url[auth_end] != '/' && url[auth_end] != '?' && url[auth_end] != '#') {
		++auth_end;
	}
	authority = url.substr(pos, auth_end - pos);
	pos = auth_end;

	// Extract path
	if (pos < url.size() && url[pos] == '/') {
		size_t path_end = pos;
		while (path_end < url.size() && url[path_end] != '?' && url[path_end] != '#') {
			++path_end;
		}
		path = url.substr(pos, path_end - pos);
		pos = path_end;
	}

	// Extract query (without '?')
	if (pos < url.size() && url[pos] == '?') {
		++pos; // skip '?'
		size_t query_end = pos;
		while (query_end < url.size() && url[query_end] != '#') {
			++query_end;
		}
		query = url.substr(pos, query_end - pos);
		pos = query_end;
	}

	// Fragment is dropped (pos may point to '#...' which we skip)

	// -----------------------------------------------------------------------
	// 2. Normalize scheme (lowercase)
	// -----------------------------------------------------------------------
	ToLower(scheme);

	// -----------------------------------------------------------------------
	// 3. Normalize authority: split into userinfo, host, port
	// -----------------------------------------------------------------------
	std::string userinfo;
	std::string host;
	std::string port;

	// Find userinfo
	size_t at_pos = authority.find('@');
	std::string host_port;
	if (at_pos != std::string::npos) {
		userinfo = authority.substr(0, at_pos + 1); // include '@'
		host_port = authority.substr(at_pos + 1);
	} else {
		host_port = authority;
	}

	// Check for IPv6 in brackets
	if (!host_port.empty() && host_port[0] == '[') {
		size_t bracket_end = host_port.find(']');
		if (bracket_end != std::string::npos) {
			host = host_port.substr(0, bracket_end + 1);
			if (bracket_end + 1 < host_port.size() && host_port[bracket_end + 1] == ':') {
				port = host_port.substr(bracket_end + 2);
			}
		} else {
			host = host_port;
		}
	} else {
		size_t colon_pos = host_port.rfind(':');
		if (colon_pos != std::string::npos) {
			// Verify what's after the colon is a valid port (all digits)
			std::string potential_port = host_port.substr(colon_pos + 1);
			bool all_digits = !potential_port.empty();
			for (char c : potential_port) {
				if (!std::isdigit(static_cast<unsigned char>(c))) {
					all_digits = false;
					break;
				}
			}
			if (all_digits) {
				host = host_port.substr(0, colon_pos);
				port = potential_port;
			} else {
				host = host_port;
			}
		} else {
			host = host_port;
		}
	}

	// Lowercase host
	ToLower(host);

	// Remove trailing dot from host (DNS canonical form)
	if (!host.empty() && host.back() == '.') {
		host.pop_back();
	}

	// Remove default port
	if (IsDefaultPort(scheme, port)) {
		port.clear();
	}

	// -----------------------------------------------------------------------
	// 4. Normalize path
	// -----------------------------------------------------------------------
	path = EnsureLeadingSlash(path);
	path = RemoveDotSegments(path);
	path = NormalizePercentEncoding(path);
	path = RemoveTrailingSlashes(path);

	// -----------------------------------------------------------------------
	// 5. Normalize query
	// -----------------------------------------------------------------------
	query = SortQueryParams(query);
	query = NormalizePercentEncoding(query);

	// -----------------------------------------------------------------------
	// 6. Reassemble
	// -----------------------------------------------------------------------
	std::string result;
	result.reserve(scheme.size() + 3 + userinfo.size() + host.size() + 1 + port.size() + path.size() + 1 +
	               query.size());

	result += scheme;
	result += "://";
	result += userinfo;
	result += host;
	if (!port.empty()) {
		result += ':';
		result += port;
	}
	result += path;
	if (!query.empty()) {
		result += '?';
		result += query;
	}

	return result;
}

} // namespace netquack
} // namespace duckdb
