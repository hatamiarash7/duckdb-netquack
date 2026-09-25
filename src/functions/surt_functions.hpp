// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
void UrlToSurtFunction(DataChunk &args, ExpressionState &state, Vector &result);
void SurtToUrlFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
std::string UrlToSurt(const std::string_view &input);
std::string SurtToUrl(const std::string_view &input);
} // namespace netquack
} // namespace duckdb
