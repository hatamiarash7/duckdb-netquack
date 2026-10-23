// Copyright 2025 Arash Hatami

#include "extract_host.hpp"

#include "../utils/url_helpers.hpp"

namespace duckdb {
void ExtractHostFunction(DataChunk &args, ExpressionState &, Vector &result) {
	const auto &input_vector = args.data[0];
	auto result_data = netquack_compat::MutableData<string_t>(result);
	auto &result_validity = netquack_compat::MutableValidity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);

		try {
			auto host = netquack::ExtractHost(input);
			result_data[i] = StringVector::AddString(result, host);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting host: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractHost(const std::string_view &input) {
	if (input.empty()) {
		return "";
	}

	const char *data = input.data();
	size_t size = input.size();

	std::string_view host = getURLHost(data, size);

	return std::string(host);
}
} // namespace netquack
} // namespace duckdb
