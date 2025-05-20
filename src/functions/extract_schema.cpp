// Copyright 2025 Arash Hatami

#include "extract_schema.hpp"

#include <regex>

namespace duckdb
{
    // Function to extract the schema from a URL
    void ExtractSchemaFunction (DataChunk &args, ExpressionState &state, Vector &result)
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
                // Extract the schema using the utility function
                auto schema    = netquack::ExtractSchema (input);
                result_data[i] = StringVector::AddString (result, schema);
            }
            catch (const std::exception &e)
            {
                result_data[i] = "Error extracting schema: " + std::string (e.what ());
            }
        };
    }

    namespace netquack
    {
        std::string ExtractSchema (const std::string &input)
        {
            // Regex to match the schema component of a URL
            // Explanation:
            // ^                - Start of the string
            // (http|https|ftp) - Capturing group for common protocols (http, https, ftp)
            // :\/\/            - Matches "://" after the protocol
            // |                - OR
            // (mailto|sms|tel) - Capturing group for other protocols (mailto, sms, tel)
            // :[^/]            - Matches ":" followed by any character except "/"
            std::regex schema_regex (R"(^(http|https|ftp|rsync):\/\/|^(mailto|sms|tel):[^/])");
            std::smatch schema_match;

            // Use regex_search to find the schema component in the input string
            if (std::regex_search (input, schema_match, schema_regex))
            {
                // Check if the schema was matched in either group
                if (schema_match.size () > 1)
                {
                    if (schema_match[1].matched)
                    {
                        // Group 1 matches "http", "https", or "ftp"
                        return schema_match[1].str ();
                    }
                    else if (schema_match[2].matched)
                    {
                        // Group 2 matches "mailto", "sms", or "tel"
                        return schema_match[2].str ();
                    }
                }
            }

            // If no schema is found, return an empty string
            return "";
        }
    } // namespace netquack
} // namespace duckdb
