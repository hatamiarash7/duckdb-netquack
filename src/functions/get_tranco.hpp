// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb::netquack {
// Function to update the Tranco list table
void UpdateTrancoListFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Function to get the Tranco rank of a domain
void GetTrancoRankFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Function to get the Tranco rank category of a domain
void GetTrancoRankCategoryFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Table function exposing the cached Tranco list (rank, domain, category)
struct TrancoListFunc {
	static unique_ptr<TableRef> BindReplace(ClientContext &context, TableFunctionBindInput &input);
};
} // namespace duckdb::netquack
