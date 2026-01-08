// Auto-generated from Mozilla Public Suffix List using gperf

#include "tld_lookup.hpp"

#include "tld_lookup_generated.hpp"

namespace duckdb
{
    namespace netquack
    {
        bool isValidTLD (const char *str, size_t len)
        {
            return TLDLookupHash::isValidTLD (str, len) != nullptr;
        }

        bool isValidTLD (const std::string &suffix)
        {
            return isValidTLD (suffix.c_str (), suffix.length ());
        }

        std::string getEffectiveTLD (const std::string &hostname)
        {
            if (hostname.empty ())
            {
                return "";
            }

            // Implement proper public suffix algorithm:
            // Find the longest matching public suffix

            // First check if the entire hostname is a TLD
            if (isValidTLD (hostname))
            {
                return hostname;
            }

            // Try all possible suffixes and find the longest match
            std::string longest_tld;

            for (size_t pos = 0; pos < hostname.length (); ++pos)
            {
                if (hostname[pos] == '.')
                {
                    std::string candidate = hostname.substr (pos + 1);
                    if (isValidTLD (candidate))
                    {
                        // Keep the longest match
                        if (candidate.length () > longest_tld.length ())
                        {
                            longest_tld = candidate;
                        }
                    }
                }
            }

            // If we found a valid TLD, return it
            if (!longest_tld.empty ())
            {
                return longest_tld;
            }

            // If no valid TLD found, return the last part after the last dot
            size_t last_dot = hostname.find_last_of ('.');
            if (last_dot != std::string::npos)
            {
                return hostname.substr (last_dot + 1);
            }

            // No dots, return entire hostname
            return hostname;
        }
    } // namespace netquack
} // namespace duckdb
