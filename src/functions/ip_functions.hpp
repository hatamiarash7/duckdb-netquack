// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Scalar function: is_valid_ip(VARCHAR) -> BOOLEAN
void IsValidIPFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: is_private_ip(VARCHAR) -> BOOLEAN
void IsPrivateIPFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ip_to_int(VARCHAR) -> UINT64
void IPToIntFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: int_to_ip(UINT64) -> VARCHAR
void IntToIPFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ip_version(VARCHAR) -> INT8
void IPVersionFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ip_in_range(VARCHAR, VARCHAR) -> BOOLEAN
void IPInRangeFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ip_to_ptr(VARCHAR) -> VARCHAR
void IPToPTRFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Check if a string is a valid IPv4 address
bool IsValidIPv4(const std::string &ip);

// Check if a string is a valid IPv6 address
bool IsValidIPv6(const std::string &ip);

// Check if an IPv4 address is in a private/reserved range
bool IsPrivateIPv4(const std::string &ip);

// Check if an IPv6 address is in a private/reserved range
bool IsPrivateIPv6(const std::string &ip);

// Convert IPv4 address to 32-bit integer
uint32_t IPv4ToInt(const std::string &ip);

// Convert IPv6 address to 128-bit integer (returned as high and low 64-bit parts)
// For simplicity we return the full 128 bits as a __uint128_t or two uint64_t
// DuckDB HUGEINT can hold 128-bit values, but we use UBIGINT for IPv4 only
uint32_t IPv4ToUint32(const std::string &ip);

// Convert 32-bit integer to IPv4 address
std::string Uint32ToIPv4(uint32_t ip);

// Detect IP version: returns 4, 6, or 0 (invalid)
int DetectIPVersion(const std::string &ip);

// Check if an IP falls within a CIDR block: returns 1 (in range), 0 (not in range), or -1 (invalid input)
int IPInRange(const std::string &ip, const std::string &cidr);

// Build the reverse DNS name (in-addr.arpa / ip6.arpa) for an IP; returns empty string for invalid input
std::string IPToPTR(const std::string &ip);
} // namespace netquack
} // namespace duckdb
