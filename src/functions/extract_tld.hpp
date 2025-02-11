#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the top-level domain from a URL
		std::string ExtractTLD(ExpressionState &state, const std::string &input);
	}
}
