// Copyright 2025 Arash Hatami

#include "extract_domain.hpp"

#include "../utils/tld_lookup.hpp"
#include "../utils/url_helpers.hpp"

namespace duckdb {
void ExtractDomainFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);

		try {
			auto domain = netquack::ExtractDomain(input);
			result_data[i] = StringVector::AddString(result, domain);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting domain: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractDomain(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	const char *data = input.data();
	size_t size = input.size();

	std::string_view host = getURLHost(data, size);

	// Handle edge cases where standard URL parsing fails
	if (host.empty()) {
		// Handle mailto: URLs - extract domain from email address
		if (input.length() > 7 && input.substr(0, 7) == "mailto:") {
			std::string email_part = input.substr(7);
			size_t at_pos = email_part.find('@');
			if (at_pos != std::string::npos && at_pos + 1 < email_part.length()) {
				std::string email_domain = email_part.substr(at_pos + 1);
				// Remove any trailing path/query/fragment
				size_t end_pos = email_domain.find_first_of("/?#");
				if (end_pos != std::string::npos) {
					email_domain = email_domain.substr(0, end_pos);
				}

				// Process the email domain directly
				std::string tld = getEffectiveTLD(email_domain);
				if (tld.empty()) {
					return email_domain;
				}

				if (email_domain == tld) {
					return email_domain;
				}

				// Extract domain.tld from email domain
				if (email_domain.length() > tld.length() &&
				    email_domain.substr(email_domain.length() - tld.length()) == tld) {
					size_t tld_start = email_domain.length() - tld.length();
					if (tld_start > 0 && email_domain[tld_start - 1] == '.') {
						size_t domain_start = email_domain.find_last_of('.', tld_start - 2);
						if (domain_start != std::string::npos) {
							return email_domain.substr(domain_start + 1);
						} else {
							return email_domain;
						}
					}
				}

				return email_domain;
			} else {
				return "";
			}
		} else {
			// Handle bare hostnames without URL structure
			bool has_protocol = input.find("://") != std::string::npos;
			bool has_path = input.find('/') != std::string::npos;
			bool has_query = input.find('?') != std::string::npos;
			bool has_fragment = input.find('#') != std::string::npos;

			if (!has_protocol && !has_path && !has_query && !has_fragment) {
				// Check for IPv6 addresses in brackets - these should return empty
				if (input.front() == '[' && input.back() == ']') {
					return "";
				}

				// Treat entire input as hostname, but strip port if present
				size_t colon_pos = input.find(':');
				size_t host_length = (colon_pos != std::string::npos) ? colon_pos : size;

				// Reject single characters as invalid hostnames
				if (host_length <= 1) {
					return "";
				}

				// Single-word hostnames: only accept valid TLDs (e.g., "com"), reject others (e.g., "localhost")
				std::string temp_host(data, host_length);
				if (temp_host.find('.') == std::string::npos) {
					// Check if it's a valid TLD (like "com"), if not reject (like "localhost")
					if (!isValidTLD(temp_host)) {
						return "";
					}
					// If it's a valid TLD, return it directly
					return temp_host;
				}

				host = std::string_view(data, host_length);
			} else {
				return "";
			}
		}
	}

	// Remove trailing dot if present
	if (host[host.size() - 1] == '.') {
		host.remove_suffix(1);
	}

	std::string host_str(host);

	// For IPv4 addresses return empty
	const char *last_dot = find_last_symbols_or_null<'.'>(host.data(), host.data() + host.size());
	if (last_dot && isNumericASCII(last_dot[1])) {
		return "";
	}

	// Apply public suffix algorithm to find longest matching TLD
	std::string tld = getEffectiveTLD(host_str);

	// If no TLD found, return entire host (for cases like single words)
	if (tld.empty()) {
		return host_str;
	}

	// If the host is just the TLD, return it
	if (host_str == tld) {
		return host_str;
	}

	// Count dots to understand structure
	size_t dot_count = 0;
	for (char c : host_str) {
		if (c == '.') {
			dot_count++;
		}
	}

	// If no dots, this is not a proper domain (like "localhost")
	if (dot_count == 0) {
		return "";
	}

	// Find where the TLD starts in the hostname
	if (host_str.length() > tld.length() && host_str.substr(host_str.length() - tld.length()) == tld) {
		// Check if there's a dot before the TLD
		size_t tld_start = host_str.length() - tld.length();
		if (tld_start > 0 && host_str[tld_start - 1] == '.') {
			// Find the domain part (one level before TLD)
			size_t domain_start = host_str.find_last_of('.', tld_start - 2);
			if (domain_start != std::string::npos) {
				return host_str.substr(domain_start + 1);
			} else {
				// No subdomain, return domain.tld
				return host_str;
			}
		}
	}

	return host_str;
}
} // namespace netquack
} // namespace duckdb
