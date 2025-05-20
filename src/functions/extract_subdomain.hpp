// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    // Function to extract the sub-domain from a URL
    void ExtractSubDomainFunction (DataChunk &args, ExpressionState &state, Vector &result);

    namespace netquack
    {
        // Function to extract the sub-domain from a URL
        std::string ExtractSubDomain (ExpressionState &state, const std::string &input);
    } // namespace netquack
} // namespace duckdb
