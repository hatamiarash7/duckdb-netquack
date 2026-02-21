// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Function to extract the fragment from a URL
void ExtractFragmentFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Function to extract the fragment from a URL
std::string ExtractFragment(const std::string &input);
} // namespace netquack
} // namespace duckdb
