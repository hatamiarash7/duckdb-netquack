// Copyright 2025 Arash Hatami

#include "extract_tld.hpp"

#include <regex>

#include "../utils/utils.hpp"

namespace duckdb
{
    // Function to extract the top-level domain from a URL
    void ExtractTLDFunction (DataChunk &args, ExpressionState &state, Vector &result)
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
                // Extract the top-level domain using the utility function
                auto tld       = netquack::ExtractTLD (state, input);
                result_data[i] = StringVector::AddString (result, tld);
            }
            catch (const std::exception &e)
            {
                result_data[i] = "Error extracting tld: " + std::string (e.what ());
            }
        }
    }

    namespace netquack
    {
        std::string ExtractTLD (ExpressionState &state, const std::string &input)
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
                    public_suffix = candidate;
                    break;
                }
            }

            return public_suffix;
        }
    } // namespace netquack
} // namespace duckdb
