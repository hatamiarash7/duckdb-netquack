// Copyright 2025 Arash Hatami

#include "extract_tld.hpp"
#include "../utils/url_helpers.hpp"
#include "../utils/tld_lookup.hpp"

namespace duckdb
{
    void ExtractTLDFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);
        auto &result_validity = FlatVector::Validity (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto value = input_vector.GetValue (i);
            if (value.IsNull ())
            {
                result_validity.SetInvalid (i);
                continue;
            }

            auto input = value.ToString ();
            std::transform (input.begin (), input.end (), input.begin (), ::tolower);

            try
            {
                auto tld       = netquack::ExtractTLD (input);
                result_data[i] = StringVector::AddString (result, tld);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting tld: " + std::string (e.what ()));
            }
        }
    }

    namespace netquack
    {
        std::string ExtractTLD (const std::string &input)
        {
            if (input.empty())
                return "";

            const char* data = input.data();
            size_t size = input.size();

            std::string_view host = getURLHost(data, size);

            // If getURLHost returns empty, try treating the entire input as a hostname
            // This handles cases like single words "com" that don't have dots
            if (host.empty())
            {
                // Check if the input looks like a simple hostname (no protocol, path, etc.)
                bool has_protocol = input.find("://") != std::string::npos;
                bool has_path = input.find('/') != std::string::npos;
                bool has_query = input.find('?') != std::string::npos;
                bool has_fragment = input.find('#') != std::string::npos;

                if (!has_protocol && !has_path && !has_query && !has_fragment)
                {
                    // Treat entire input as hostname, but strip port if present
                    size_t colon_pos = input.find(':');
                    size_t host_length = (colon_pos != std::string::npos) ? colon_pos : size;

                    // Reject single characters as invalid hostnames
                    if (host_length <= 1)
                    {
                        return "";
                    }

                    host = std::string_view(data, host_length);
                }
                else
                {
                    return "";
                }
            }

            // Remove trailing dot if present
            if (host[host.size() - 1] == '.')
                host.remove_suffix(1);

            std::string host_str(host);

            // For IPv4 addresses return empty
            const char* last_dot = find_last_symbols_or_null<'.'>(host.data(), host.data() + host.size());
            if (last_dot && isNumericASCII(last_dot[1]))
                return "";

            // Use the proper TLD lookup to get the effective TLD
            std::string effective_tld = getEffectiveTLD(host_str);

            // If the effective TLD is empty, try the last part
            if (effective_tld.empty())
            {
                size_t last_dot = host_str.find_last_of('.');
                if (last_dot != std::string::npos)
                {
                    return host_str.substr(last_dot + 1);
                }
                return host_str; // No dots, return entire string
            }

            return effective_tld;
        }
    } // namespace netquack
} // namespace duckdb
