#include "extract_query.hpp"

#include <regex>

namespace duckdb
{
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
