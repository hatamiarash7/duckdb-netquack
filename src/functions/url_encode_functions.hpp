// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
void UrlEncodeFunction(DataChunk &args, ExpressionState &state, Vector &result);
void UrlDecodeFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
std::string UrlEncode(const std::string &input);
std::string UrlDecode(const std::string &input);
} // namespace netquack
} // namespace duckdb
