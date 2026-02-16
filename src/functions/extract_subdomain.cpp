// Copyright 2025 Arash Hatami

#include "extract_subdomain.hpp"

#include "../utils/tld_lookup.hpp"
#include "../utils/url_helpers.hpp"

namespace duckdb {
void ExtractSubDomainFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
			auto subdomain = netquack::ExtractSubDomain(input);
			result_data[i] = StringVector::AddString(result, subdomain);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting subdomain: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractSubDomain(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	const char *data = input.data();
	size_t size = input.size();

	std::string_view host = getURLHost(data, size);

	if (host.empty()) {
		return "";
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

	// Get the effective TLD
	std::string tld = getEffectiveTLD(host_str);
	if (tld.empty()) {
		{ return ""; } // No TLD found
	}

	// If the host is just the TLD, no subdomain
	if (host_str == tld) {
		{ return ""; }
	}

	// Find where the TLD starts in the hostname
	if (host_str.length() > tld.length() && host_str.substr(host_str.length() - tld.length()) == tld) {
		// Check if there's a dot before the TLD
		size_t tld_start = host_str.length() - tld.length();
		if (tld_start > 0 && host_str[tld_start - 1] == '.') {
			// Find the domain part (one level before TLD)
			size_t domain_start = host_str.find_last_of('.', tld_start - 2);
			if (domain_start != std::string::npos) {
				// There's a subdomain - return everything before domain
				return host_str.substr(0, domain_start);
			} else {
				// No subdomain, just domain.tld
				{ return ""; }
			}
		}
	}

	{ return ""; }
}
} // namespace netquack
} // namespace duckdb
