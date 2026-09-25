// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// Scalar function: extract_urls(VARCHAR) -> VARCHAR[]
void ExtractURLsFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: extract_domains(VARCHAR) -> VARCHAR[]
void ExtractDomainsFunction(DataChunk &args, ExpressionState &state, Vector &result);

// Scalar function: extract_ips(VARCHAR) -> VARCHAR[]
void ExtractIPsFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
// Find every scheme://... URL in free text, in order of appearance
std::vector<std::string> ExtractURLs(const std::string_view &input);

// Find every domain name with a known TLD in free text (lowercased), in order of appearance
std::vector<std::string> ExtractDomains(const std::string_view &input);

// Find every valid IPv4 and IPv6 address in free text, in order of appearance
std::vector<std::string> ExtractIPs(const std::string_view &input);
} // namespace netquack
} // namespace duckdb
