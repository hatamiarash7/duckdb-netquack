// Auto-generated from Mozilla Public Suffix List using gperf

#pragma once

#include <cstring>
#include <string>

namespace duckdb::netquack {
// Check if a suffix is a valid public suffix (TLD) using perfect hash
bool isValidTLD(const char *str, size_t len);
bool isValidTLD(const std::string &suffix);

// Get the effective TLD for a hostname
std::string getEffectiveTLD(const std::string &hostname);

// Check a normalized (lowercase, no leading/trailing dot) name against the full PSL rules,
// including wildcard (*.ck) and exception (!www.ck) rules
bool isPublicSuffix(const std::string &name);

// Check if a normalized single label is a TLD listed in the PSL (e.g. com, uk, ck)
bool isKnownTLD(const std::string &label);
} // namespace duckdb::netquack
