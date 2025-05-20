// Copyright 2025 Arash Hatami

#include "extract_extension.hpp"

#include <regex>

namespace duckdb
{
    // Function to extract the extension from a URL
    void ExtractExtensionFunction (DataChunk &args, ExpressionState &state, Vector &result)
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
                // Extract the extension using the utility function
                auto ext       = netquack::ExtractExtension (input);
                result_data[i] = StringVector::AddString (result, ext);
            }
            catch (const std::exception &e)
            {
                result_data[i] = "Error extracting extension: " + std::string (e.what ());
            }
        };
    }

    namespace netquack
    {
        std::string ExtractExtension (const std::string &input)
        {
            // Regex to extract valid file extensions from paths/URLs
            // Explanation:
            // (?<!\.)                - Negative lookbehind ensures no preceding dot (avoids "..ext")
            // \.                     - Literal dot (extension separator)
            // ([a-zA-Z0-9]{1,10})    - Capturing group for extension:
            //                         - 1-10 alphanumeric chars (prevents long garbage matches)
            // (?=[?#]|$)             - Positive lookahead for:
            //                         - Query separator (?)
            //                         - Fragment (#)
            //                         - Or end of string ($)
            //
            // Examples matched:
            //   /path/image.jpg      -> jpg
            //   /doc.v12.pdf         -> pdf
            //   /archive.tar.gz      -> gz
            //   https://site.com/page.html?param=1 -> html
            //
            // Rejected cases:
            //   /path..jpg           -> no match (double dot)
            //   /path.               -> no match (no extension after dot)
            //   /.hidden_file        -> no match (no alnum after dot)
            //   /path.with.dots/file -> no match (not at end)
            std::regex ext_regex (R"(^(?!.*\.\.)(?:.*\/)?[^\/?#]+\.([a-zA-Z0-9]{1,10})(?=[?#]|$))");
            std::smatch ext_match;

            // Use regex_search to find the extension component in the input string
            if (std::regex_search (input, ext_match, ext_regex))
            {
                // Check if the extension group was matched and is not empty
                if (ext_match.size () > 1 && ext_match[1].matched)
                {
                    return ext_match[1].str ();
                }
            }

            // If no extension is found, return an empty string
            return "";
        }
    } // namespace netquack
} // namespace duckdb
