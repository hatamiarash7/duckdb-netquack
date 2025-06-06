// Copyright 2025 Arash Hatami

#include "extract_subdomain.hpp"

#include <regex>

#include "../utils/utils.hpp"

namespace duckdb
{
    // Function to extract the sub-domain from a URL
    void ExtractSubDomainFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        // Extract the input from the arguments
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto input = input_vector.GetValue (i).ToString ();
            std::transform (input.begin (), input.end (), input.begin (), ::tolower);

            try
            {
                // Extract the subdomain using the utility function
                auto subdomain = netquack::ExtractSubDomain (state, input);
                result_data[i] = StringVector::AddString (result, subdomain);
            }
            catch (const std::exception &e)
            {
                // Set NULL on error
                FlatVector::SetNull (result, i, true);
            }
        }
    }

    namespace netquack
    {
        std::string ExtractSubDomain (ExpressionState &state, const std::string &input)
        {
            // Load the public suffix list if not already loaded
            auto &db = *state.GetContext ().db;
            netquack::LoadPublicSuffixList (db, false);
            Connection con (db);

            // Extract the host from the URL
            // This regex captures the host, excluding protocol, path, query, fragment, and port.
            // It explicitly excludes '/', '\s', '#', '?', ':' from the host.
            std::regex host_regex (R"(^(?:(?:https?|ftp|rsync):\/\/)?([^\/\s#?:]+))");
            std::smatch host_match;
            std::string host_str;

            // No need for searchable_input, regex_search can take input directly
            if (std::regex_search (input, host_match, host_regex) && host_match.size () > 1)
            {
                host_str = host_match[1].str ();
            }
            else
            {
                return ""; // No host found
            }

            // Split the host into parts
            std::vector<std::string> parts;
            std::istringstream stream (host_str);
            std::string part;
            while (std::getline (stream, part, '.'))
            {
                parts.push_back (part);
            }

            // Find the longest matching public suffix
            std::string public_suffix;
            int public_suffix_index = -1;

            // Iterate through all possible suffix combinations, from shortest to longest.
            // The goal is to find the longest known public suffix.
            // For example, for 'a.b.c.co.uk', it will test:
            // uk, co.uk, c.co.uk, b.c.co.uk, a.b.c.co.uk
            // If 'co.uk' is a public suffix, it will be matched.
            // If 'c.co.uk' is also a public suffix (e.g. *.sch.uk), that would be matched.
            // The last and longest match is chosen.
            // The current logic takes the *first* match from the right that is a PSL entry.
            for (size_t j = 0; j < parts.size (); j++)
            {
                // Build the candidate suffix
                std::string candidate;
                for (size_t k = j; k < parts.size (); k++)
                {
                    candidate += (k == j ? "" : ".") + parts[k];
                }

                // Query the public suffix list
                auto query        = "SELECT 1 FROM public_suffix_list WHERE suffix = '" + candidate + "'";
                auto query_result = con.Query (query);

                if (query_result->RowCount () > 0)
                {
                    public_suffix       = candidate;
                    public_suffix_index = j;
                    break;
                }
            }

            // Determine the subdomain
            std::string subdomain;
            if (!public_suffix.empty () && public_suffix_index > 0)
            {
                // Combine all parts before the public suffix
                for (int i = 0; i < public_suffix_index - 1; i++)
                {
                    subdomain += (i == 0 ? "" : ".") + parts[i];
                }
            }

            return subdomain;
        }
    } // namespace netquack
} // namespace duckdb
