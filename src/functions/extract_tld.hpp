// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Function to extract the top-level domain from a URL
void ExtractTLDFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Function to extract the top-level domain from a URL
std::string ExtractTLD(const std::string &input);
} // namespace netquack
} // namespace duckdb
