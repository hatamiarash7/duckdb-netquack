// Copyright 2025 Arash Hatami

#include "extract_query.hpp"
#include "../utils/url_helpers.hpp"

namespace duckdb
{
    void ExtractQueryStringFunction (DataChunk &args, ExpressionState &state, Vector &result)
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

            try
            {
                auto query_string = netquack::ExtractQueryString (input);
                result_data[i]    = StringVector::AddString (result, query_string);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting query string: " + std::string (e.what ()));
            }
        }
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
            const char* query_start = find_first_symbols<'?'>(pos, end);
            if (query_start == end)
                return "";

            // Find the fragment '#' character - must check from beginning
            const char* fragment = find_first_symbols<'#'>(pos, end);

            // If '#' comes before '?', then '?' is part of fragment, not query
            if (fragment < query_start)
                return "";

            // Skip the '?' character
            ++query_start;

            // Calculate query size (up to fragment or end)
            const char* query_end = (fragment != end) ? fragment : end;
            size_t query_size = query_end - query_start;
            if (query_size == 0)
                return "";

            return std::string(query_start, query_size);
        }
    } // namespace netquack
} // namespace duckdb
