#include "extract_query.hpp"

#include <regex>

namespace duckdb
{
	// Function to extract the query string from a URL
	void ExtractQueryStringFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto url = url_vector.GetValue(0).ToString();

		// Extract the query string
		auto query_string = netquack::ExtractQueryString(url);

		// Set the result
		result.SetValue(0, Value(query_string));
	}

	namespace netquack
	{
		std::string ExtractQueryString(const std::string &input)
		{
			// Regex to match the query string component of a URL
			// Explanation:
			// (?:\?|&)  - Non-capturing group to match either "?" (start of query) or "&" (query parameter separator)
			// ([^#]+)   - Capturing group to match the query string (any characters except "#")
			std::regex query_regex(R"((?:\?|&)([^#]+))");
			std::smatch query_match;

			// Use regex_search to find the query string in the input
			if (std::regex_search(input, query_match, query_regex))
			{
				// Check if the query string group was matched and is not empty
				if (query_match.size() > 1 && query_match[1].matched)
				{
					return query_match[1].str();
				}
			}

			// If no query string is found, return an empty string
			return "";
		}
	}
}
