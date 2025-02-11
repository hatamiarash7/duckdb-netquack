#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the sub-domain from a URL
		std::string ExtractSubDomain(ExpressionState &state, const std::string &input);
	}
}
