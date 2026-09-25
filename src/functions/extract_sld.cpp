// Copyright 2026 Arash Hatami

#include "extract_sld.hpp"

#include "../utils/tld_lookup.hpp"
#include "extract_domain.hpp"

namespace duckdb {
void ExtractSLDFunction(DataChunk &args, ExpressionState &, Vector &result) {
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
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);

		try {
			auto sld = netquack::ExtractSLD(input);
			result_data[i] = StringVector::AddString(result, sld);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting SLD: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractSLD(const std::string &input) {
	std::string domain = ExtractDomain(input);
	if (domain.empty()) {
		return "";
	}

	std::string tld = getEffectiveTLD(domain);
	if (tld.empty() || tld.length() + 1 >= domain.length()) {
		return "";
	}

	size_t tld_start = domain.length() - tld.length();
	if (domain[tld_start - 1] != '.' || domain.compare(tld_start, tld.length(), tld) != 0) {
		return "";
	}

	return domain.substr(0, tld_start - 1);
}
} // namespace netquack
} // namespace duckdb
