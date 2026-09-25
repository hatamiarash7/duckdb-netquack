// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
void DefangFunction(DataChunk &args, ExpressionState &state, Vector &result);
void RefangFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
std::string Defang(const std::string_view &input);
std::string Refang(const std::string_view &input);
} // namespace netquack
} // namespace duckdb
