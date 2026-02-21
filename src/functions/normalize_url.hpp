// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
void NormalizeURLFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Normalize a URL: lowercase scheme/host, remove default ports, sort query params,
// remove trailing slashes, remove fragments, decode unnecessary percent-encoding,
// and remove dot segments from the path.
std::string NormalizeURL(const std::string &input);
} // namespace netquack
} // namespace duckdb
