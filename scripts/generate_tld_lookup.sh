#!/usr/bin/env bash

# Download the public suffix list if not present
if [ ! -f public_suffix_list.dat ]; then
    echo "Downloading public suffix list..."
    wget -nv -O public_suffix_list.dat https://publicsuffix.org/list/public_suffix_list.dat
fi

# Generate C++ header with TLD lookup
echo "Generating TLD lookup header..."

# Generate gperf file like ClickHouse does
echo '%language=C++
%define lookup-function-name isValidTLD
%define class-name TLDLookupHash
%readonly-tables
%includes
%compare-strncmp
%{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wunused-macros"
// NOLINTBEGIN(modernize-macro-to-enum)
%}
# List generated using https://publicsuffix.org/list/public_suffix_list.dat
%%' > src/utils/tld_lookup.gperf

# Extract ALL TLDs from the public suffix list (both single and multi-part)
# Remove comments, empty lines, and wildcards
grep -v "^//" public_suffix_list.dat | \
grep -v "^$" | \
grep -v "^\!" | \
grep -v "^\*" | \
sed 's/^[[:space:]]*//' | \
sort -u >> src/utils/tld_lookup.gperf

echo "%%" >> src/utils/tld_lookup.gperf
echo '// NOLINTEND(modernize-macro-to-enum)
#pragma GCC diagnostic pop' >> src/utils/tld_lookup.gperf

# Generate perfect hash function using gperf (required)
if ! command -v gperf >/dev/null 2>&1; then
    echo "Error: gperf is required for optimal TLD lookup generation"
    echo "Please install gperf: brew install gperf (macOS) or apt-get install gperf (Linux)"
    exit 1
fi

echo "Generating perfect hash function with gperf..."
gperf src/utils/tld_lookup.gperf > src/utils/tld_lookup_generated.hpp

# Remove deprecated 'register' storage class specifier for C++17 compatibility
echo "Removing deprecated 'register' keywords for C++17 compatibility..."
sed -i.bak 's/register const char \*/const char */g; s/register unsigned int/unsigned int/g' src/utils/tld_lookup_generated.hpp
rm -f src/utils/tld_lookup_generated.hpp.bak

# Create header file
cat > src/utils/tld_lookup.hpp << 'EOF'
// Auto-generated from Mozilla Public Suffix List using gperf

#pragma once

#include <string>
#include <cstring>

namespace duckdb
{
    namespace netquack
    {
        // Check if a suffix is a valid public suffix (TLD) using perfect hash
        bool isValidTLD(const char* str, size_t len);
        bool isValidTLD(const std::string& suffix);
        
        // Get the effective TLD for a hostname
        std::string getEffectiveTLD(const std::string& hostname);
    }
}
EOF

# Create implementation file
cat > src/utils/tld_lookup.cpp << 'EOF'
// Auto-generated from Mozilla Public Suffix List using gperf

#include "tld_lookup.hpp"
#include "tld_lookup_generated.hpp"

namespace duckdb
{
    namespace netquack
    {
        bool isValidTLD(const char* str, size_t len)
        {
            return TLDLookupHash::isValidTLD(str, len) != nullptr;
        }

        bool isValidTLD(const std::string& suffix)
        {
            return isValidTLD(suffix.c_str(), suffix.length());
        }

        std::string getEffectiveTLD(const std::string& hostname)
        {
            if (hostname.empty())
                return "";

            // Implement proper public suffix algorithm:
            // Find the longest matching public suffix
            
            // First check if the entire hostname is a TLD
            if (isValidTLD(hostname))
            {
                return hostname;
            }
            
            // Try all possible suffixes and find the longest match
            std::string longest_tld;
            
            for (size_t pos = 0; pos < hostname.length(); ++pos)
            {
                if (hostname[pos] == '.')
                {
                    std::string candidate = hostname.substr(pos + 1);
                    if (isValidTLD(candidate))
                    {
                        // Keep the longest match
                        if (candidate.length() > longest_tld.length())
                        {
                            longest_tld = candidate;
                        }
                    }
                }
            }
            
            // If we found a valid TLD, return it
            if (!longest_tld.empty())
            {
                return longest_tld;
            }
            
            // If no valid TLD found, return the last part after the last dot
            size_t last_dot = hostname.find_last_of('.');
            if (last_dot != std::string::npos)
            {
                return hostname.substr(last_dot + 1);
            }
            
            // No dots, return entire hostname
            return hostname;
        }
    }
}
EOF

echo "TLD lookup files generated successfully!"
