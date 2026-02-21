// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Function to encode a string to Base64
void Base64EncodeFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Function to decode a Base64 string
void Base64DecodeFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Encode a string to Base64
std::string Base64Encode(const std::string &input);

// Decode a Base64 string
std::string Base64Decode(const std::string &input);
} // namespace netquack
} // namespace duckdb
