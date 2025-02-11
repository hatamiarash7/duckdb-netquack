#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    void ExtractDomainFunction (DataChunk &args, ExpressionState &state, Vector &result);

    namespace netquack
    {
        // Function to extract the main domain from a URL
        std::string ExtractDomain (ExpressionState &state, const std::string &input);
    } // namespace netquack
} // namespace duckdb
