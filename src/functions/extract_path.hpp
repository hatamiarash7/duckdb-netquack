#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    // Function to extract the path from a URL
    void ExtractPathFunction (DataChunk &args, ExpressionState &state, Vector &result);

    namespace netquack
    {
        // Function to extract the path from a URL or host
        std::string ExtractPath (const std::string &input);
    } // namespace netquack
} // namespace duckdb
