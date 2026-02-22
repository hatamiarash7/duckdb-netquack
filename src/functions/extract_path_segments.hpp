// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
namespace netquack {

// Table function to split a URL path into individual segment rows
struct ExtractPathSegmentsFunc {
	static unique_ptr<FunctionData> Bind(ClientContext &context, TableFunctionBindInput &input,
	                                     vector<LogicalType> &return_types, vector<string> &names);
	static unique_ptr<LocalTableFunctionState> InitLocal(ExecutionContext &context, TableFunctionInitInput &input,
	                                                     GlobalTableFunctionState *global_state_p);
	static OperatorResultType Function(ExecutionContext &context, TableFunctionInput &data_p, DataChunk &input,
	                                   DataChunk &output);
};

// Pure logic: extract path segments from a URL
std::vector<std::string> ExtractPathSegments(const std::string &input);

} // namespace netquack
} // namespace duckdb
