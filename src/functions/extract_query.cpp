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

// Bind data (no state needed at bind time for in-out functions)
struct ExtractQueryParametersData : public TableFunctionData {};

// Local state to track parsed parameters and current output position
struct ExtractQueryParametersLocalState : public LocalTableFunctionState {
	std::vector<std::pair<std::string, std::string>> parameters;
	idx_t current_idx = 0;
	bool done = false;
};

unique_ptr<FunctionData> ExtractQueryParametersFunc::Bind(ClientContext &context, TableFunctionBindInput &input,
                                                          vector<LogicalType> &return_types, vector<string> &names) {
	// Define output columns: key and value
	return_types.emplace_back(LogicalType(LogicalTypeId::VARCHAR));
	names.emplace_back("key");

	return_types.emplace_back(LogicalType(LogicalTypeId::VARCHAR));
	names.emplace_back("value");

	return make_uniq<ExtractQueryParametersData>();
}

unique_ptr<LocalTableFunctionState> ExtractQueryParametersFunc::InitLocal(ExecutionContext &context,
                                                                          TableFunctionInitInput &input,
                                                                          GlobalTableFunctionState *global_state_p) {
	return make_uniq<ExtractQueryParametersLocalState>();
}

OperatorResultType ExtractQueryParametersFunc::Function(ExecutionContext &context, TableFunctionInput &data_p,
                                                        DataChunk &input, DataChunk &output) {
	auto &local_state = data_p.local_state->Cast<ExtractQueryParametersLocalState>();

	// Already finished outputting for this input row — request next input
	if (local_state.done) {
		local_state.done = false;
		local_state.parameters.clear();
		local_state.current_idx = 0;
		return OperatorResultType::NEED_MORE_INPUT;
	}

	// If we have remaining parameters from a previous HAVE_MORE_OUTPUT, continue outputting
	if (!local_state.parameters.empty() && local_state.current_idx < local_state.parameters.size()) {
		idx_t count = 0;
		while (local_state.current_idx < local_state.parameters.size() && count < STANDARD_VECTOR_SIZE) {
			auto &param = local_state.parameters[local_state.current_idx];
			output.data[0].SetValue(count, Value(param.first));
			output.data[1].SetValue(count, Value(param.second));
			++local_state.current_idx;
			++count;
		}
		output.SetCardinality(count);

		if (local_state.current_idx >= local_state.parameters.size()) {
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
	std::string query_string = ExtractQueryString(url);
	local_state.parameters = ParseQueryParameters(query_string);
	local_state.current_idx = 0;

	if (local_state.parameters.empty()) {
		output.SetCardinality(0);
		return OperatorResultType::NEED_MORE_INPUT;
	}

	// Output as many parameters as we can
	idx_t count = 0;
	while (local_state.current_idx < local_state.parameters.size() && count < STANDARD_VECTOR_SIZE) {
		auto &param = local_state.parameters[local_state.current_idx];
		output.data[0].SetValue(count, Value(param.first));
		output.data[1].SetValue(count, Value(param.second));
		++local_state.current_idx;
		++count;
	}
	output.SetCardinality(count);

	if (local_state.current_idx >= local_state.parameters.size()) {
		// All parameters output — mark done so next call requests new input
		local_state.done = true;
	}
	return OperatorResultType::HAVE_MORE_OUTPUT;
}
} // namespace netquack
} // namespace duckdb
