// Copyright 2026 Arash Hatami

#include "domain_depth.hpp"

#include "../utils/url_helpers.hpp"

namespace duckdb {
void DomainDepthFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<int32_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();

		try {
			result_data[i] = netquack::DomainDepth(input);
		} catch (const std::exception &) {
			result_data[i] = 0;
		}
	}
}

namespace netquack {
int32_t DomainDepth(const std::string &input) {
	if (input.empty()) {
		return 0;
	}

	// Lowercase the input for consistent host extraction
	std::string lowered = input;
	std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

	// Extract the host from the URL using the existing utility
	const char *data = lowered.data();
	size_t size = lowered.size();
	std::string_view host = getURLHost(data, size);

	// If no host could be extracted, treat the whole input as a potential bare domain
	std::string host_str;
	if (host.empty()) {
		// If the input has a scheme (://), there's no valid host — return 0
		if (lowered.find("://") != std::string::npos) {
			return 0;
		}

		// Strip any path/query/fragment from bare domain input
		std::string bare = lowered;
		auto slash_pos = bare.find('/');
		if (slash_pos != std::string::npos) {
			bare = bare.substr(0, slash_pos);
		}
		auto question_pos = bare.find('?');
		if (question_pos != std::string::npos) {
			bare = bare.substr(0, question_pos);
		}
		auto hash_pos = bare.find('#');
		if (hash_pos != std::string::npos) {
			bare = bare.substr(0, hash_pos);
		}
		// Strip port
		auto colon_pos = bare.rfind(':');
		if (colon_pos != std::string::npos) {
			bool all_digits = true;
			for (size_t k = colon_pos + 1; k < bare.size(); ++k) {
				if (!std::isdigit(static_cast<unsigned char>(bare[k]))) {
					all_digits = false;
					break;
				}
			}
			if (all_digits && colon_pos + 1 < bare.size()) {
				bare = bare.substr(0, colon_pos);
			}
		}

		if (bare.empty()) {
			return 0;
		}

		// Check if it contains at least one dot
		bool has_dot = false;
		for (char c : bare) {
			if (c == '.') {
				has_dot = true;
			}
			if (c == ' ' || c == '\t' || c == '<' || c == '>') {
				return 0;
			}
		}
		if (!has_dot) {
			// Single label like "localhost" — depth is 1
			// But only if it looks reasonable (alphanumeric + hyphens)
			for (char c : bare) {
				if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
					continue;
				}
				return 0;
			}
			return 1;
		}
		host_str = bare;
	} else {
		host_str = std::string(host);
	}

	if (host_str.empty()) {
		return 0;
	}

	// Remove trailing dot if present (DNS canonical form)
	if (!host_str.empty() && host_str.back() == '.') {
		host_str.pop_back();
	}

	if (host_str.empty()) {
		return 0;
	}

	// Skip IPv6 addresses (e.g., [::1]) — they have no domain depth
	if (host_str.front() == '[') {
		return 0;
	}

	// Skip IPv4 addresses (all parts are numeric)
	{
		bool all_numeric_parts = true;
		size_t part_start = 0;
		int dot_count = 0;
		for (size_t j = 0; j <= host_str.size(); ++j) {
			if (j == host_str.size() || host_str[j] == '.') {
				if (j == part_start) {
					all_numeric_parts = false;
					break;
				}
				for (size_t k = part_start; k < j; ++k) {
					if (!std::isdigit(static_cast<unsigned char>(host_str[k]))) {
						all_numeric_parts = false;
						break;
					}
				}
				if (!all_numeric_parts) {
					break;
				}
				if (host_str[j] == '.') {
					++dot_count;
				}
				part_start = j + 1;
			}
		}
		if (all_numeric_parts && dot_count == 3) {
			return 0; // IPv4 address, not a domain
		}
	}

	// Count dot-separated levels
	int32_t depth = 1;
	for (char c : host_str) {
		if (c == '.') {
			++depth;
		}
	}

	return depth;
}
} // namespace netquack
} // namespace duckdb
