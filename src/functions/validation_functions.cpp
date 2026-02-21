// Copyright 2026 Arash Hatami

#include "validation_functions.hpp"

#include "../utils/url_helpers.hpp"

#include <algorithm>
#include <string>

namespace duckdb {

void IsValidURLFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<bool>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		result_data[i] = netquack::IsValidURL(input);
	}
}

void IsValidDomainFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<bool>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		result_data[i] = netquack::IsValidDomain(input);
	}
}

namespace netquack {

// ---------------------------------------------------------------------------
// Domain validation (RFC 1035 / RFC 1123)
// ---------------------------------------------------------------------------
// Rules:
// - Total length: 1..253 characters
// - Each label separated by '.'
// - Each label: 1..63 characters
// - Labels contain only ASCII letters, digits, and hyphens
// - Labels must not start or end with a hyphen
// - Must have at least one dot (i.e., at least two labels)
// - TLD must not be all-numeric
// - No empty labels (consecutive dots)
// - Optional trailing dot is allowed (FQDN notation)
// ---------------------------------------------------------------------------
bool IsValidDomain(const std::string &input) {
	if (input.empty()) {
		return false;
	}

	// Work on a copy, strip optional trailing dot
	std::string domain = input;
	if (domain.back() == '.') {
		domain.pop_back();
	}

	if (domain.empty() || domain.size() > 253) {
		return false;
	}

	// Split into labels
	size_t label_count = 0;
	size_t label_start = 0;
	std::string last_label;

	for (size_t i = 0; i <= domain.size(); i++) {
		if (i == domain.size() || domain[i] == '.') {
			size_t label_len = i - label_start;

			// Empty label (consecutive dots or leading dot)
			if (label_len == 0) {
				return false;
			}

			// Label too long
			if (label_len > 63) {
				return false;
			}

			// Check label characters: alphanumeric and hyphens only
			for (size_t j = label_start; j < i; j++) {
				char c = domain[j];
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-')) {
					return false;
				}
			}

			// Label must not start or end with hyphen
			if (domain[label_start] == '-' || domain[i - 1] == '-') {
				return false;
			}

			last_label = domain.substr(label_start, label_len);
			label_count++;
			label_start = i + 1;
		}
	}

	// Must have at least two labels (e.g., "example.com")
	if (label_count < 2) {
		return false;
	}

	// TLD must not be all-numeric
	bool all_numeric = true;
	for (char c : last_label) {
		if (c < '0' || c > '9') {
			all_numeric = false;
			break;
		}
	}
	if (all_numeric) {
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------
// URL validation
// ---------------------------------------------------------------------------
// A well-formed URL must have:
// - A valid scheme (letters, digits, +, -, . — starting with a letter)
// - "://" separator after the scheme
// - A non-empty authority (host) section
// - The host must be a valid domain, IPv4, or bracketed IPv6
// - Optional port, path, query, and fragment
// ---------------------------------------------------------------------------
bool IsValidURL(const std::string &input) {
	if (input.empty()) {
		return false;
	}

	const char *data = input.data();
	size_t size = input.size();
	const char *pos = data;
	const char *end = data + size;

	// --- Parse scheme ---
	// Scheme must start with a letter
	if (!isAlphaASCII(*pos)) {
		return false;
	}

	const char *scheme_start = pos;
	++pos;
	while (pos < end && (isAlphaNumericASCII(*pos) || *pos == '+' || *pos == '-' || *pos == '.')) {
		++pos;
	}

	// Must have "://" after scheme
	if (pos + 2 >= end || *pos != ':' || *(pos + 1) != '/' || *(pos + 2) != '/') {
		return false;
	}

	size_t scheme_len = pos - scheme_start;
	if (scheme_len == 0) {
		return false;
	}

	pos += 3; // skip "://"

	// --- Parse authority (host[:port]) ---
	if (pos >= end) {
		return false; // Empty authority
	}

	// Skip optional userinfo (user:pass@)
	const char *authority_start = pos;
	const char *at_pos = nullptr;
	for (const char *scan = pos; scan < end && *scan != '/' && *scan != '?' && *scan != '#'; ++scan) {
		if (*scan == '@') {
			at_pos = scan;
			break;
		}
	}
	if (at_pos) {
		pos = at_pos + 1;
		if (pos >= end || *pos == '/' || *pos == '?' || *pos == '#') {
			return false; // Empty host after userinfo
		}
	}

	// --- Parse host ---
	const char *host_start = pos;
	const char *host_end = nullptr;

	// Check for IPv6 bracketed address
	if (*pos == '[') {
		const char *bracket_close = find_first_symbols<']'>(pos, end);
		if (bracket_close == end) {
			return false; // Unclosed bracket
		}
		// Extract the IPv6 address between brackets
		std::string ipv6_addr(pos + 1, bracket_close);
		if (ipv6_addr.empty()) {
			return false;
		}
		// Basic IPv6 validation: must contain at least one ':'
		if (ipv6_addr.find(':') == std::string::npos) {
			return false;
		}
		pos = bracket_close + 1;
		host_end = pos;
	} else {
		// Regular host (domain or IPv4)
		while (pos < end && *pos != ':' && *pos != '/' && *pos != '?' && *pos != '#') {
			++pos;
		}
		host_end = pos;

		if (host_end == host_start) {
			return false; // Empty host
		}

		std::string host(host_start, host_end);

		// Check if it looks like an IPv4 address
		bool looks_like_ipv4 = true;
		for (char c : host) {
			if (c != '.' && (c < '0' || c > '9')) {
				looks_like_ipv4 = false;
				break;
			}
		}

		if (looks_like_ipv4) {
			// Validate as IPv4: exactly 4 octets, each 0-255
			int dot_count = 0;
			int octet_start_idx = 0;
			bool valid_ipv4 = true;
			for (size_t i = 0; i <= host.size() && valid_ipv4; i++) {
				if (i == host.size() || host[i] == '.') {
					int len = static_cast<int>(i) - octet_start_idx;
					if (len == 0 || len > 3) {
						valid_ipv4 = false;
						break;
					}
					int val = 0;
					for (int j = octet_start_idx; j < static_cast<int>(i); j++) {
						val = val * 10 + (host[j] - '0');
					}
					if (val > 255) {
						valid_ipv4 = false;
						break;
					}
					if (host[i] == '.') {
						dot_count++;
					}
					octet_start_idx = static_cast<int>(i) + 1;
				}
			}
			if (!valid_ipv4 || dot_count != 3) {
				return false;
			}
		} else {
			// Validate as domain name (relaxed: allow single-label hosts like localhost)
			// Each label: alphanumeric + hyphens, no start/end with hyphen, 1-63 chars
			size_t lbl_start = 0;
			for (size_t i = 0; i <= host.size(); i++) {
				if (i == host.size() || host[i] == '.') {
					size_t lbl_len = i - lbl_start;
					if (lbl_len == 0 || lbl_len > 63) {
						return false;
					}
					for (size_t j = lbl_start; j < i; j++) {
						char c = host[j];
						if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-')) {
							return false;
						}
					}
					if (host[lbl_start] == '-' || host[i - 1] == '-') {
						return false;
					}
					lbl_start = i + 1;
				}
			}
		}
	}

	// --- Optional port ---
	if (pos < end && *pos == ':') {
		++pos;
		if (pos >= end || *pos == '/' || *pos == '?' || *pos == '#') {
			return false; // Empty port after colon
		}
		while (pos < end && *pos >= '0' && *pos <= '9') {
			++pos;
		}
		// After digits, must be /, ?, #, or end
		if (pos < end && *pos != '/' && *pos != '?' && *pos != '#') {
			return false; // Non-digit in port
		}
	}

	// --- Rest of URL (path, query, fragment) ---
	// We accept the rest as-is; main validation is scheme + authority
	return true;
}

} // namespace netquack
} // namespace duckdb
