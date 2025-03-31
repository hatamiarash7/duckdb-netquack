#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    // Function to extract the extension from a URL
    void ExtractExtensionFunction (DataChunk &args, ExpressionState &state, Vector &result);

    namespace netquack
    {
        // Function to extract the extension from a URL or host
        std::string ExtractExtension (const std::string &input);
    } // namespace netquack
} // namespace duckdb
