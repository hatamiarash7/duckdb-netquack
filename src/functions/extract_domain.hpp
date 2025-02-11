#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the main domain from a URL
		std::string ExtractDomain(ExpressionState &state, const std::string &input);
	}
}
