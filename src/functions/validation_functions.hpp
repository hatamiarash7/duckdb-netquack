// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Scalar function: is_valid_url(VARCHAR) -> BOOLEAN
void IsValidURLFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: is_valid_domain(VARCHAR) -> BOOLEAN
void IsValidDomainFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Check if a string is a well-formed URL
bool IsValidURL(const std::string &input);

// Validate a domain name against RFC 1035 / RFC 1123 rules
bool IsValidDomain(const std::string &input);
} // namespace netquack
} // namespace duckdb
