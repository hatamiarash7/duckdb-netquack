#include "extract_host.hpp"

#include <regex>

namespace duckdb
{
	namespace netquack
	{
		std::string ExtractHost(const std::string &input)
		{
			// Regex to match the host component of a URL
			// Explanation:
			// ^                - Start of the string
			// (?:              - Non-capturing group for the optional protocol
			//   https?:\/\/    - Matches "http://" or "https://"
			// )?
			// ([^\/\s:?#]+)    - Capturing group for the host (any characters except '/', ':', '?', '#', or whitespace)
			std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\s:?#]+))");
			std::smatch host_match;

			// Use regex_search to find the host component in the input string
			if (std::regex_search(input, host_match, host_regex))
			{
				// Check if the host group was matched and is not empty
				if (host_match.size() > 1 && host_match[1].matched)
				{
					return host_match[1].str();
				}
			}

			// If no host is found, return an empty string
			return "";
		}
	}
}
