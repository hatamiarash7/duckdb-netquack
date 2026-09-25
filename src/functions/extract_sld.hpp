// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Function to extract the second-level domain label from a URL
void ExtractSLDFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Function to extract the label just before the public suffix
std::string ExtractSLD(const std::string &input);
} // namespace netquack
} // namespace duckdb
