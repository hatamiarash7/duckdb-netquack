// Copyright 2025 Arash Hatami

#include "extract_domain.hpp"

#include <regex>

#include "../utils/utils.hpp"

namespace duckdb
{
    // Function to extract the domain from a URL
    void ExtractDomainFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        // Extract the input from the arguments
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);

        // Load the public suffix list if not already loaded
        auto &db = *state.GetContext ().db;
        netquack::LoadPublicSuffixList (db, false);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto input = input_vector.GetValue (i).ToString ();
            std::transform (input.begin (), input.end (), input.begin (), ::tolower);

            try
            {
                // Extract the domain using the utility function
                auto domain    = netquack::ExtractDomain (state, input);
                result_data[i] = StringVector::AddString (result, domain);
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
        std::string ExtractDomain (ExpressionState &state, const std::string &input)
        {
            auto &db = *state.GetContext ().db;
            Connection con (db);

            // Extract the host from the URL
            // This regex captures the host, excluding protocol, path, query, fragment, and port.
            // It explicitly excludes '/', '\s', '#', '?', ':' from the host.
            std::regex host_regex (R"(^(?:(?:https?|ftp|rsync):\/\/)?([^\/\s#?:]+))");
            std::smatch host_match;
            std::string host_str; // Use a separate string for the matched host

            // Search for the host in the input string
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
            int public_suffix_index = -1; // Using -1 to indicate no valid public suffix part found yet

            // Iterate through all possible suffix combinations, from shortest to longest.
            // The goal is to find the longest known public suffix.
            // For example, for 'a.b.c.co.uk', it will test:
            // uk, co.uk, c.co.uk, b.c.co.uk, a.b.c.co.uk
            // If 'co.uk' is a public suffix, it will be matched.
            // If 'c.co.uk' is also a public suffix (e.g. *.sch.uk), that would be matched.
            // The last and longest match is chosen by breaking after the first DB match,
            // assuming suffixes are ordered or queried appropriately by the PSL logic.
            // However, the original loop structure implies checking all parts and
            // the longest one that is a PSL entry should be chosen.
            // The current logic takes the *first* match from the right that is a PSL entry.
            // Let's refine the comment to reflect the actual loop behavior.

            // Iterate through parts of the hostname from right to left to find the longest public suffix.
            // For 'a.b.c.co.uk', it will form candidates:
            // 1. uk
            // 2. co.uk
            // 3. c.co.uk
            // 4. b.c.co.uk
            // 5. a.b.c.co.uk
            // It stops at the first and longest valid suffix found in the public_suffix_list.
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

            // Determine the main domain
            std::string domain;
            if (!public_suffix.empty () && public_suffix_index > 0)
            {
                // Combine the part before the public suffix with the public suffix
                domain = parts[public_suffix_index - 1] + "." + public_suffix;
            }
            else if (!public_suffix.empty ())
            {
                // No part before the suffix, use the public suffix only
                domain = public_suffix;
            }

            return domain;
        }
    } // namespace netquack
} // namespace duckdb
