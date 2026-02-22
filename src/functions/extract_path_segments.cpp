// Copyright 2026 Arash Hatami

#include "extract_path_segments.hpp"

#include "../utils/url_helpers.hpp"

#include <string>
#include <vector>

namespace duckdb {
namespace netquack {

// ---------------------------------------------------------------------------
// Pure logic: extract path from URL, then split into non-empty segments
// ---------------------------------------------------------------------------
std::vector<std::string> ExtractPathSegments(const std::string &input) {
	std::vector<std::string> segments;

	if (input.empty()) {
		return segments;
	}

	const char *data = input.data();
	size_t size = input.size();
	const char *pos = data;
	const char *end = pos + size;

	// Locate the path start (same logic as ExtractPath)
	pos = find_first_symbols<'/'>(pos, end);
	if (end == pos) {
		return segments;
	}

	bool has_subsequent_slash = pos + 1 < end && pos[1] == '/';
	if (has_subsequent_slash) {
		pos = find_first_symbols<'/'>(pos + 2, end);
		if (end == pos) {
			return segments;
		}
	}

	// Path ends at '?' or '#'
	const char *path_end = find_first_symbols<'?', '#'>(pos, end);

	// Skip leading '/'
	if (pos < path_end && *pos == '/') {
		++pos;
	}

	// Split on '/'
	const char *seg_start = pos;
	for (const char *cur = pos; cur <= path_end; ++cur) {
		if (cur == path_end || *cur == '/') {
			if (cur > seg_start) {
				segments.emplace_back(seg_start, cur - seg_start);
			}
			seg_start = cur + 1;
		}
	}

	return segments;
}

// ---------------------------------------------------------------------------
// Table function callbacks
// ---------------------------------------------------------------------------
struct ExtractPathSegmentsData : public TableFunctionData {};

struct ExtractPathSegmentsLocalState : public LocalTableFunctionState {
	std::vector<std::string> segments;
	idx_t current_idx = 0;
	bool done = false;
};

unique_ptr<FunctionData> ExtractPathSegmentsFunc::Bind(ClientContext &context, TableFunctionBindInput &input,
                                                       vector<LogicalType> &return_types, vector<string> &names) {
	// Output columns: segment_index (1-based) and segment
	return_types.emplace_back(LogicalType(LogicalTypeId::INTEGER));
	names.emplace_back("segment_index");

	return_types.emplace_back(LogicalType(LogicalTypeId::VARCHAR));
	names.emplace_back("segment");

	return make_uniq<ExtractPathSegmentsData>();
}

unique_ptr<LocalTableFunctionState> ExtractPathSegmentsFunc::InitLocal(ExecutionContext &context,
                                                                       TableFunctionInitInput &input,
                                                                       GlobalTableFunctionState *global_state_p) {
	return make_uniq<ExtractPathSegmentsLocalState>();
}

OperatorResultType ExtractPathSegmentsFunc::Function(ExecutionContext &context, TableFunctionInput &data_p,
                                                     DataChunk &input, DataChunk &output) {
	auto &local_state = data_p.local_state->Cast<ExtractPathSegmentsLocalState>();

	// Already finished outputting for this input row — request next input
	if (local_state.done) {
		local_state.done = false;
		local_state.segments.clear();
		local_state.current_idx = 0;
		return OperatorResultType::NEED_MORE_INPUT;
	}

	// Continue outputting remaining segments from a previous HAVE_MORE_OUTPUT
	if (!local_state.segments.empty() && local_state.current_idx < local_state.segments.size()) {
		idx_t count = 0;
		while (local_state.current_idx < local_state.segments.size() && count < STANDARD_VECTOR_SIZE) {
			output.data[0].SetValue(count, Value::INTEGER(static_cast<int32_t>(local_state.current_idx + 1)));
			output.data[1].SetValue(count, Value(local_state.segments[local_state.current_idx]));
			++local_state.current_idx;
			++count;
		}
		output.SetCardinality(count);

		if (local_state.current_idx >= local_state.segments.size()) {
			local_state.done = true;
		}
		return OperatorResultType::HAVE_MORE_OUTPUT;
	}

	// Parse the URL from the input chunk
	auto url_value = input.data[0].GetValue(0);
	if (url_value.IsNull()) {
		output.SetCardinality(0);
		return OperatorResultType::NEED_MORE_INPUT;
	}

	auto url = url_value.GetValue<string>();
	local_state.segments = ExtractPathSegments(url);
	local_state.current_idx = 0;

	if (local_state.segments.empty()) {
		output.SetCardinality(0);
		return OperatorResultType::NEED_MORE_INPUT;
	}

	// Output as many segments as we can
	idx_t count = 0;
	while (local_state.current_idx < local_state.segments.size() && count < STANDARD_VECTOR_SIZE) {
		output.data[0].SetValue(count, Value::INTEGER(static_cast<int32_t>(local_state.current_idx + 1)));
		output.data[1].SetValue(count, Value(local_state.segments[local_state.current_idx]));
		++local_state.current_idx;
		++count;
	}
	output.SetCardinality(count);

	if (local_state.current_idx >= local_state.segments.size()) {
		local_state.done = true;
	}
	return OperatorResultType::HAVE_MORE_OUTPUT;
}

} // namespace netquack
} // namespace duckdb
