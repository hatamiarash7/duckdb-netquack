// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/execution/expression_executor.hpp"

namespace duckdb {
void ExtractDomainFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Function to extract the main domain from a URL
std::string ExtractDomain(const std::string &input);
} // namespace netquack
} // namespace duckdb
