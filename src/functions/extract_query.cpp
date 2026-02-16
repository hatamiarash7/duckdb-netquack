// Copyright 2025 Arash Hatami

#include "extract_query.hpp"

#include <utility>
#include <vector>

#include "../utils/url_helpers.hpp"

namespace duckdb {
void ExtractQueryStringFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
			auto query_string = netquack::ExtractQueryString(input);
			result_data[i] = StringVector::AddString(result, query_string);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error extracting query string: " + std::string(e.what()));
		}
	}
}

namespace netquack {
std::string ExtractQueryString(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	const char *data = input.data();
	size_t size = input.size();
	const char *pos = data;
	const char *end = pos + size;

	// Find the '?' character
	const char *query_start = find_first_symbols<'?'>(pos, end);
	if (query_start == end) {
		return "";
	}

	// Find the fragment '#' character - must check from beginning
	const char *fragment = find_first_symbols<'#'>(pos, end);

	// If '#' comes before '?', then '?' is part of fragment, not query
	if (fragment < query_start) {
		return "";
	}

	// Skip the '?' character
	++query_start;

	// Calculate query size (up to fragment or end)
	const char *query_end = (fragment != end) ? fragment : end;
	size_t query_size = query_end - query_start;
	if (query_size == 0) {
		return "";
	}

	return std::string(query_start, query_size);
}

// Parse query string into key-value pairs
static std::vector<std::pair<std::string, std::string>> ParseQueryParameters(const std::string &query_string) {
	std::vector<std::pair<std::string, std::string>> params;

	if (query_string.empty()) {
		return params;
	}

	const char *pos = query_string.data();
	const char *end = pos + query_string.size();

	while (pos < end) {
		// Find the next '&' or end
		const char *amp = pos;
		while (amp < end && *amp != '&') {
			++amp;
		}

		// Now we have a key=value pair from pos to amp
		const char *eq = pos;
		while (eq < amp && *eq != '=') {
			++eq;
		}

		std::string key;
		std::string value;

		if (eq < amp) {
			// Found '='
			key = std::string(pos, eq - pos);
			value = std::string(eq + 1, amp - (eq + 1));
		} else {
			// No '=' found, entire segment is key with empty value
			key = std::string(pos, amp - pos);
			value = "";
		}

		if (!key.empty()) {
			params.emplace_back(std::move(key), std::move(value));
		}

		// Move past the '&'
		pos = amp + 1;
	}

	return params;
}

// Data structure to hold the parsed query parameters
struct ExtractQueryParametersData : public TableFunctionData {
	std::string url;
	std::vector<std::pair<std::string, std::string>> parameters;
};

// Global state to track scanning position
struct ExtractQueryParametersGlobalState : public GlobalTableFunctionState {
	idx_t current_idx = 0;
	std::mutex lock;
};

// Local state (not used but required)
struct ExtractQueryParametersLocalState : public LocalTableFunctionState {};

unique_ptr<FunctionData> ExtractQueryParametersFunc::Bind(ClientContext &context, TableFunctionBindInput &input,
                                                          vector<LogicalType> &return_types, vector<string> &names) {
	// Define output columns: key and value
	return_types.emplace_back(LogicalType(LogicalTypeId::VARCHAR));
	names.emplace_back("key");

	return_types.emplace_back(LogicalType(LogicalTypeId::VARCHAR));
	names.emplace_back("value");

	auto result = make_uniq<ExtractQueryParametersData>();

	// Get the URL from input
	if (!input.inputs.empty() && !input.inputs[0].IsNull()) {
		result->url = input.inputs[0].GetValue<string>();
		// Extract query string and parse parameters
		std::string query_string = ExtractQueryString(result->url);
		result->parameters = ParseQueryParameters(query_string);
	}

	return std::move(result);
}

unique_ptr<GlobalTableFunctionState> ExtractQueryParametersFunc::InitGlobal(ClientContext &context,
                                                                            TableFunctionInitInput &input) {
	return make_uniq<ExtractQueryParametersGlobalState>();
}

unique_ptr<LocalTableFunctionState> ExtractQueryParametersFunc::InitLocal(ExecutionContext &context,
                                                                          TableFunctionInitInput &input,
                                                                          GlobalTableFunctionState *global_state_p) {
	return make_uniq<ExtractQueryParametersLocalState>();
}

void ExtractQueryParametersFunc::Scan(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<ExtractQueryParametersData>();
	auto &global_state = data_p.global_state->Cast<ExtractQueryParametersGlobalState>();

	idx_t count = 0;
	idx_t max_count = STANDARD_VECTOR_SIZE;

	lock_guard<mutex> guard(global_state.lock);

	while (global_state.current_idx < data.parameters.size() && count < max_count) {
		auto &param = data.parameters[global_state.current_idx];

		output.data[0].SetValue(count, Value(param.first));
		output.data[1].SetValue(count, Value(param.second));

		++global_state.current_idx;
		++count;
	}

	output.SetCardinality(count);
}
} // namespace netquack
} // namespace duckdb
