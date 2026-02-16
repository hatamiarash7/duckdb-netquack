// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
namespace netquack {
// Function to update the Tranco list table
void UpdateTrancoListFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Function to get the Tranco rank of a domain
void GetTrancoRankFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Function to get the Tranco rank category of a domain
void GetTrancoRankCategoryFunction(DataChunk &args, ExpressionState &state, Vector &result);
} // namespace netquack
} // namespace duckdb
