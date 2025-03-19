#pragma once

#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "duckdb.hpp"

namespace duckdb
{
    namespace netquack
    {
        struct IPCalcFunc
        {
            static unique_ptr<FunctionData> Bind (ClientContext &context, TableFunctionBindInput &input, vector<LogicalType> &return_types, vector<string> &names);
            static unique_ptr<LocalTableFunctionState> InitLocal (ExecutionContext &context, TableFunctionInitInput &input, GlobalTableFunctionState *global_state_p);
            static OperatorResultType Function (ExecutionContext &context, TableFunctionInput &data_p, DataChunk &input, DataChunk &output);
        };
    } // namespace netquack
} // namespace duckdb
