// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Function to extract the host from a URL
void ExtractHostFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Function to extract the host from a URL
std::string ExtractHost(const std::string &input);
} // namespace netquack
} // namespace duckdb
