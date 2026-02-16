// Auto-generated from Mozilla Public Suffix List using gperf

#pragma once

#include <cstring>
#include <string>

namespace duckdb {
namespace netquack {
// Check if a suffix is a valid public suffix (TLD) using perfect hash
bool isValidTLD(const char *str, size_t len);
bool isValidTLD(const std::string &suffix);

// Get the effective TLD for a hostname
std::string getEffectiveTLD(const std::string &hostname);
} // namespace netquack
} // namespace duckdb
