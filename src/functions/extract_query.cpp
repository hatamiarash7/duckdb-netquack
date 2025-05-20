// Copyright 2025 Arash Hatami

#include "extract_query.hpp"

#include <regex>

namespace duckdb
{
    // Function to extract the query string from a URL
    void ExtractQueryStringFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        // Extract the input from the arguments
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto input = input_vector.GetValue (i).ToString ();

            try
            {
                // Extract the query string using the utility function
                auto query_string = netquack::ExtractQueryString (input);
                result_data[i]    = StringVector::AddString (result, query_string);
            }
            catch (const std::exception &e)
            {
                result_data[i] = "Error extracting query string: " + std::string (e.what ());
            }
        };
    }

    namespace netquack
    {
        std::string ExtractQueryString (const std::string &input)
        {
            // Regex to match the query string component of a URL
            // Explanation:
            // (?:\?|&)  - Non-capturing group to match either "?" (start of query) or "&" (query parameter separator)
            // ([^#]+)   - Capturing group to match the query string (any characters except "#")
            std::regex query_regex (R"((?:\?|&)([^#]+))");
            std::smatch query_match;

            // Use regex_search to find the query string in the input
            if (std::regex_search (input, query_match, query_regex))
            {
                // Check if the query string group was matched and is not empty
                if (query_match.size () > 1 && query_match[1].matched)
                {
                    return query_match[1].str ();
                }
            }

            // If no query string is found, return an empty string
            return "";
        }
    } // namespace netquack
} // namespace duckdb
