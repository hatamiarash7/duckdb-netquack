// Copyright 2025 Arash Hatami

#include "extract_path.hpp"

#include "../utils/url_helpers.hpp"

namespace duckdb {
void ExtractPathFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
			auto path = netquack::ExtractPath(input);
			result_data[i] = StringVector::AddString(result, path);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting path: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractPath(const std::string &input) {
	if (input.empty()) {
		return "/";
	}

	const char *data = input.data();
	size_t size = input.size();
	const char *pos = data;
	const char *end = pos + size;

	// We support URLs with and without schema:
	// 1. http://host/path
	// 2. host/path
	// We search for first slash and if there is subsequent slash, then skip and repeat search for the next slash.

	pos = find_first_symbols<'/'>(pos, end);
	if (end == pos) {
		return "/";
	}

	bool has_subsequent_slash = pos + 1 < end && pos[1] == '/';
	if (has_subsequent_slash) {
		// Search for next slash.
		pos = find_first_symbols<'/'>(pos + 2, end);
		if (end == pos) {
			return "/";
		}
	}

	// Extract path without query string or fragment
	const char *query_string_or_fragment = find_first_symbols<'?', '#'>(pos, end);
	size_t path_size = query_string_or_fragment - pos;

	if (path_size == 0) {
		return "/";
	}

	return std::string(pos, path_size);
}
} // namespace netquack
} // namespace duckdb
