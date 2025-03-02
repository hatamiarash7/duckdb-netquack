#include "extract_path.hpp"

#include <regex>

namespace duckdb
{
    // Function to extract the path from a URL
    void ExtractPathFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        // Extract the input from the arguments
        auto &input_vector = args.data[0];
        auto input         = input_vector.GetValue (0).ToString ();

        // Extract the path using the utility function
        auto path = netquack::ExtractPath (input);

        // Set the result
        result.SetValue (0, Value (path));
    }

    namespace netquack
    {
        std::string ExtractPath (const std::string &input)
        {
            // Regex to match the path component of a URL
            // Explanation:
            // ^                - Start of the string
            // (?:              - Non-capturing group for the protocol and domain part
            //   (?:(?:ftp|https?|rsync):\/\/)? - Optional ftp://, http://, https://, or rsync://
            //   (?:[^\/\s]+)    - Domain name (any characters except '/' or whitespace)
            // )
            // (\/[^?#]*)       - Capturing group for the path (starts with '/', followed by any characters except '?' or '#')
            std::regex path_regex (R"(^(?:(?:(?:ftp|https?|rsync):\/\/)?(?:[^\/\s]+))(\/[^?#]*))");
            std::smatch path_match;

            // Use regex_search to find the path component in the input string
            if (std::regex_search (input, path_match, path_regex))
            {
                // Check if the path group was matched and is not empty
                if (path_match.size () > 1 && path_match[1].matched)
                {
                    return path_match[1].str ();
                }
            }

            // If no path is found, return the default path "/"
            return "/";
        }
    } // namespace netquack
} // namespace duckdb
