// Copyright 2025 Arash Hatami

#include "extract_query.hpp"
#include "../utils/url_helpers.hpp"

namespace duckdb
{
    void ExtractQueryStringFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto input = input_vector.GetValue (i).ToString ();

            try
            {
                auto query_string = netquack::ExtractQueryString (input);
                result_data[i]    = StringVector::AddString (result, query_string);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting query string: " + std::string (e.what ()));
            }
        };
    }

    namespace netquack
    {
        std::string ExtractQueryString (const std::string &input)
        {
            if (input.empty())
                return "";

            const char* data = input.data();
            size_t size = input.size();
            const char* pos = data;
            const char* end = pos + size;

            // Find the '?' character
            pos = find_first_symbols<'?'>(pos, end);
            if (pos == end)
                return "";

            // Skip the '?' character
            ++pos;

            // Find the fragment '#' character
            const char* fragment = find_first_symbols<'#'>(pos, end);
            
            size_t query_size = fragment - pos;
            if (query_size == 0)
                return "";
                
            return std::string(pos, query_size);
        }
    } // namespace netquack
} // namespace duckdb
