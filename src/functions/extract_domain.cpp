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
        auto input         = input_vector.GetValue (0).ToString ();

        if (input.empty ())
        {
            result.SetValue (0, Value (""));
            return;
        }

        // Extract the domain using the utility function
        auto domain = netquack::ExtractDomain (state, input);

        result.SetValue (0, Value (domain));
    }

    namespace netquack
    {
        std::string ExtractDomain (ExpressionState &state, const std::string &input)
        {
            // Load the public suffix list if not already loaded
            auto &db = *state.GetContext ().db;
            netquack::LoadPublicSuffixList (db, false);
            Connection con (db);

            // Extract the host from the URL
            std::regex host_regex (R"(^(?:(?:https?|ftp|rsync):\/\/)?([^\/\?:]+))");
            std::smatch host_match;
            if (!std::regex_search (input, host_match, host_regex))
            {
                return "";
            }

            auto host = host_match[1].str ();

            // Split the host into parts
            std::vector<std::string> parts;
            std::istringstream stream (host);
            std::string part;
            while (std::getline (stream, part, '.'))
            {
                parts.push_back (part);
            }

            // Find the longest matching public suffix
            std::string public_suffix;
            int public_suffix_index = -1;

            for (int j = 0; j < parts.size (); j++)
            {
                // Build the candidate suffix
                std::string candidate;
                for (int k = j; k < parts.size (); k++)
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
