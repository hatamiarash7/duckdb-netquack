#include "extract_port.hpp"

#include <regex>

namespace duckdb
{
    // Function to extract the port from a URL
    void ExtractPortFunction (DataChunk &args, ExpressionState &state, Vector &result)
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
                // Extract the port using the utility function
                auto port      = netquack::ExtractPort (input);
                result_data[i] = StringVector::AddString (result, port);
            }
            catch (const std::exception &e)
            {
                result_data[i] = "Error extracting port: " + std::string (e.what ());
            }
        };
    }

    namespace netquack
    {
        std::string ExtractPort (const std::string &input)
        {
            // Regex to match the port component of a URI/URL in all common cases
            // Explanation:
            // (?:^|(?<=\]|[\w.-])[:]) - Port prefix matcher (non-capturing group):
            //   ^                     - Start of string (for non-URL cases like "localhost:8080")
            //   |                     - OR
            //   (?<=\]|[\w.-])        - Positive lookbehind for either:
            //     \]                  - Closing bracket (for IPv6 addresses like "[::1]:8080")
            //     |                   - OR
            //     [\w.-]+             - Word chars, dots, or hyphens (for hostnames/IPv4)
            //   [:]                   - Colon that precedes the port
            //
            // (\d+)                   - Capturing group for the port number (1+ digits)
            //
            // (?=[/\?#]|$)            - Positive lookahead ensures port is followed by:
            //   /                     - Path separator OR
            //   \?                    - Query separator OR
            //   #                     - Fragment separator OR
            //   $                     - End of string
            //
            // |                       - ALTERNATIVE PATTERN for auth-containing URIs:
            // (?<=@[\w.-]+:)          - Positive lookbehind for auth patterns:
            //   @                     - Auth separator
            //   [\w.-]+               - Auth credentials (user:pass)
            //   :                     - Colon before port
            // \d+                     - Port number (1+ digits)
            // (?=[/\?#]|$)            - Same lookahead as above
            std::regex port_regex (R"((?:^|\]|[\w.-]+):(\d+)(?=[\/\?#]|$))");
            std::smatch port_match;

            // Use regex_search to find the port component in the input string
            if (std::regex_search (input, port_match, port_regex))
            {
                // Check if the port group was matched and is not empty
                if (port_match.size () > 1 && port_match[1].matched)
                {
                    return port_match[1].str ();
                }
            }

            // If no port is found, return an empty string
            return "";
        }
    } // namespace netquack
} // namespace duckdb
