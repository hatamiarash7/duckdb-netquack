// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    // Function to extract the query string from a URL
    void ExtractQueryStringFunction (DataChunk &args, ExpressionState &state, Vector &result);

    namespace netquack
    {
        // Function to extract the query string from a URL
        std::string ExtractQueryString (const std::string &input);

        // Table function to extract query parameters as key-value pairs
        struct ExtractQueryParametersFunc
        {
            static unique_ptr<FunctionData> Bind (ClientContext &context, TableFunctionBindInput &input, vector<LogicalType> &return_types, vector<string> &names);
            static unique_ptr<GlobalTableFunctionState> InitGlobal (ClientContext &context, TableFunctionInitInput &input);
            static unique_ptr<LocalTableFunctionState> InitLocal (ExecutionContext &context, TableFunctionInitInput &input, GlobalTableFunctionState *global_state_p);
            static void Scan (ClientContext &context, TableFunctionInput &data_p, DataChunk &output);
        };
    } // namespace netquack
} // namespace duckdb
