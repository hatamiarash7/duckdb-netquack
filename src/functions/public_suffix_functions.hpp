// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Scalar function: is_public_suffix(VARCHAR) -> BOOLEAN
void IsPublicSuffixFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: is_known_tld(VARCHAR) -> BOOLEAN
void IsKnownTLDFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Check if a domain is exactly a public suffix (e.g. com, co.uk, github.io)
bool IsPublicSuffix(const std::string_view &input);

// Check if a single label is a top-level domain listed in the Public Suffix List
bool IsKnownTLD(const std::string_view &input);
} // namespace netquack
} // namespace duckdb
