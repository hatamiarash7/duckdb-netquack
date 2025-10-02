// Copyright 2025 Arash Hatami

#include "extract_extension.hpp"
#include "../utils/url_helpers.hpp"

namespace duckdb
{
    void ExtractExtensionFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto input = input_vector.GetValue (i).ToString ();
            std::transform (input.begin (), input.end (), input.begin (), ::tolower);

            try
            {
                auto ext       = netquack::ExtractExtension (input);
                result_data[i] = StringVector::AddString (result, ext);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting extension: " + std::string (e.what ()));
            }
        };
    }

    namespace netquack
    {
        std::string ExtractExtension (const std::string &input)
        {
            if (input.empty())
                return "";

            const char* data = input.data();
            size_t size = input.size();
            const char* pos = data;
            const char* end = pos + size;

            // Find the path part by looking for the first slash after protocol
            pos = find_first_symbols<'/'>(pos, end);
            if (pos == end)
                return "";

            bool has_subsequent_slash = pos + 1 < end && pos[1] == '/';
            if (has_subsequent_slash)
            {
                // Search for next slash (start of path)
                pos = find_first_symbols<'/'>(pos + 2, end);
                if (pos == end)
                    return "";
            }

            // Now pos points to the start of the path
            // Find the end of the path (before query or fragment)
            const char* path_end = find_first_symbols<'?', '#'>(pos, end);

            // Find the last slash in the path to get the filename
            const char* last_slash = find_last_symbols_or_null<'/'>(pos, path_end);
            const char* filename_start = last_slash ? last_slash + 1 : pos;

            // Find the last dot in the filename
            const char* last_dot = find_last_symbols_or_null<'.'>(filename_start, path_end);
            if (!last_dot || last_dot == filename_start)
                return "";

            // Check if there's a previous dot (avoid double dots like ..ext)
            if (last_dot > filename_start && *(last_dot - 1) == '.')
                return "";

            // Extract extension
            const char* ext_start = last_dot + 1;
            size_t ext_length = path_end - ext_start;

            // Validate extension (only alphanumeric, max 10 chars)
            if (ext_length == 0 || ext_length > 10)
                return "";

            for (size_t i = 0; i < ext_length; ++i)
            {
                if (!isAlphaNumericASCII(ext_start[i]))
                    return "";
            }

            return std::string(ext_start, ext_length);
        }
    } // namespace netquack
} // namespace duckdb
