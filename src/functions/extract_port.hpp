// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Function to extract the port from a URL
void ExtractPortFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Function to extract the port from a URL or host
std::string ExtractPort(const std::string &input);
} // namespace netquack
} // namespace duckdb
