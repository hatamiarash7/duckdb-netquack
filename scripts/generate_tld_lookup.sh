#!/usr/bin/env bash

set -euo pipefail

# Download the public suffix list if not present
if [ ! -f public_suffix_list.dat ]; then
	echo "Downloading public suffix list..."
	curl -fsSL https://publicsuffix.org/list/public_suffix_list.dat -o public_suffix_list.dat
fi

# Generate C++ header
echo "Generating header..."

# Generate gperf file with public suffixes
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
%%' >src/utils/tld_lookup.gperf

# Extract all records from the public suffix list (both single and multi-part)
# Remove comments, empty lines, and wildcards
grep -v '^\s*//' public_suffix_list.dat | # comments
	grep -v '^\s*$' |                        # empty lines
	grep -v '^\s*!' |                        # exceptions
	grep -v '^\s*\*' |                       # wildcards
	sed 's/^[[:space:]]*//' |                # trim left
	tr '[:upper:]' '[:lower:]' |             # normalize
	LC_ALL=C sort -u >>src/utils/tld_lookup.gperf

echo "%%" >>src/utils/tld_lookup.gperf
echo '// NOLINTEND(modernize-macro-to-enum)
#pragma GCC diagnostic pop' >>src/utils/tld_lookup.gperf

# Generate perfect hash function using gperf (required)
if ! command -v gperf >/dev/null 2>&1; then
	echo "Error: gperf is required for optimal TLD lookup generation"
	echo "Please install gperf: brew install gperf (macOS) or apt-get install gperf (Linux)"
	exit 1
fi

echo "Generating perfect hash function with gperf..."
gperf --language=C++ \
	--compare-strncmp \
	--readonly-tables \
	--switch=1 \
	--pic \
	--multiple-iterations=10 \
	src/utils/tld_lookup.gperf >src/utils/tld_lookup_generated.hpp

# Remove deprecated 'register' storage class specifier for C++17 compatibility
echo "Removing deprecated 'register' keywords for C++17 compatibility..."
sed -i.bak 's/register const char \*/const char */g; s/register unsigned int/unsigned int/g' src/utils/tld_lookup_generated.hpp
rm -f src/utils/tld_lookup_generated.hpp.bak

echo "TLD lookup files generated successfully!"
