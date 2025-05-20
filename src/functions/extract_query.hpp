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
    } // namespace netquack
} // namespace duckdb
