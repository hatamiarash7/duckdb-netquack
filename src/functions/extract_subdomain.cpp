#include "extract_domain.hpp"
#include "../utils/utils.hpp"

#include <regex>

namespace duckdb
{
	namespace netquack
	{
		std::string ExtractSubDomain(ExpressionState &state, const std::string &input)
		{
			// Load the public suffix list if not already loaded
			auto &db = *state.GetContext().db;
			netquack::LoadPublicSuffixList(db, false);
			Connection con(db);

			// Extract the host from the URL
			std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\?:]+))");
			std::smatch host_match;
			if (!std::regex_search(input, host_match, host_regex))
			{
				return "";
			}

			auto host = host_match[1].str();

			// Split the host into parts
			std::vector<std::string> parts;
			std::istringstream stream(host);
			std::string part;
			while (std::getline(stream, part, '.'))
			{
				parts.push_back(part);
			}

			// Find the longest matching public suffix
			std::string public_suffix;
			int public_suffix_index = -1;

			for (int j = 0; j < parts.size(); j++)
			{
				// Build the candidate suffix
				std::string candidate;
				for (int k = j; k < parts.size(); k++)
				{
					candidate += (k == j ? "" : ".") + parts[k];
				}

				// Query the public suffix list
				auto query = "SELECT 1 FROM public_suffix_list WHERE suffix = '" + candidate + "'";
				auto query_result = con.Query(query);

				if (query_result->RowCount() > 0)
				{
					public_suffix = candidate;
					public_suffix_index = j;
					break;
				}
			}

			// Determine the subdomain
			std::string subdomain;
			if (!public_suffix.empty() && public_suffix_index > 0)
			{
				// Combine all parts before the public suffix
				for (int i = 0; i < public_suffix_index - 1; i++)
				{
					subdomain += (i == 0 ? "" : ".") + parts[i];
				}
			}

			return subdomain;
		}
	}
}
