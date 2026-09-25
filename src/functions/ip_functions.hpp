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

// Scalar function: ipv6_compress(VARCHAR) -> VARCHAR
void IPv6CompressFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ipv6_expand(VARCHAR) -> VARCHAR
void IPv6ExpandFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: is_ipv4_mapped(VARCHAR) -> BOOLEAN
void IsIPv4MappedFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ip_type(VARCHAR) -> VARCHAR
void IPTypeFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: is_bogon(VARCHAR) -> BOOLEAN
void IsBogonFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: ip_anonymize(VARCHAR[, INTEGER, INTEGER]) -> VARCHAR
void IPAnonymizeFunction(DataChunk &args, ExpressionState &state, Vector &result);

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

// Format an IPv6 address in RFC 5952 canonical form; returns empty string for invalid or IPv4 input
std::string IPv6Compress(const std::string &ip);

// Format an IPv6 address as eight zero-padded hex groups; returns empty string for invalid or IPv4 input
std::string IPv6Expand(const std::string &ip);

// Check if an address is IPv4-mapped IPv6 (::ffff:0:0/96): returns 1 (mapped), 0 (not mapped), or -1 (invalid input)
int IsIPv4Mapped(const std::string &ip);

// Classify an IP as public, private, loopback, link_local, multicast, cgnat, documentation or reserved;
// returns empty string for invalid input
std::string IPType(const std::string &ip);

// Zero all bits after the given prefix lengths (IPv4-mapped IPv6 uses the IPv4 prefix);
// returns empty string for invalid input or out-of-range prefixes
std::string IPAnonymize(const std::string &ip, int ipv4_prefix, int ipv6_prefix);
} // namespace netquack
} // namespace duckdb
