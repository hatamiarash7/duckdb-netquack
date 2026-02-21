// Copyright 2026 Arash Hatami

#include "extract_fragment.hpp"

#include "../utils/url_helpers.hpp"

namespace duckdb {
void ExtractFragmentFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
			auto fragment = netquack::ExtractFragment(input);
			result_data[i] = StringVector::AddString(result, fragment);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting fragment: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractFragment(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	const char *data = input.data();
	size_t size = input.size();
	const char *pos = data;
	const char *end = pos + size;

	// Find the '#' character
	const char *hash_pos = find_first_symbols<'#'>(pos, end);
	if (hash_pos == end) {
		// No fragment found
		return "";
	}

	// Skip the '#' character
	++hash_pos;

	// Everything after '#' is the fragment
	size_t fragment_size = end - hash_pos;
	if (fragment_size == 0) {
		return "";
	}

	return std::string(hash_pos, fragment_size);
}
} // namespace netquack
} // namespace duckdb
