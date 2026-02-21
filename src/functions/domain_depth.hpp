// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Scalar function: domain_depth(VARCHAR) -> INTEGER
void DomainDepthFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Return the number of dot-separated levels in a domain/host
int32_t DomainDepth(const std::string &input);
} // namespace netquack
} // namespace duckdb
