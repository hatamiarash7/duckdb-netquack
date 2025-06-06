// Copyright 2025 Arash Hatami

#include "extract_host.hpp"

#include <regex>

namespace duckdb
{
    // Function to extract the host from a URL
    void ExtractHostFunction (DataChunk &args, ExpressionState &state, Vector &result)
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
                // Extract the host using the utility function
                auto host      = netquack::ExtractHost (input);
                result_data[i] = StringVector::AddString (result, host);
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
        std::string ExtractHost (const std::string &input)
        {
            // Regex to match the host component of a URL
            // Explanation:
            // ^                        - Start of the string
            // (?:                      - Non-capturing group for the optional protocol
            //   (?:ftp|https?|rsync)   - Non-capturing group for the scheme
            //   :\/\/                  - Matches "://"
            // )?
            // ([^\/\s:?#]+)            - Capturing group for the host (any characters except '/', ':', '?', '#', or whitespace)
            std::regex host_regex (R"(^(?:(?:ftp|https?|rsync):\/\/)?([^\/\s:?#]+))");
            std::smatch host_match;

            // Use regex_search to find the host component in the input string
            if (std::regex_search (input, host_match, host_regex))
            {
                // Check if the host group was matched and is not empty
                if (host_match.size () > 1 && host_match[1].matched)
                {
                    return host_match[1].str ();
                }
            }

            // If no host is found, return an empty string
            return "";
        }
    } // namespace netquack
} // namespace duckdb
