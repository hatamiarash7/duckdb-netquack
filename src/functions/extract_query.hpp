#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the query string from a URL
		std::string ExtractQueryString(const std::string &input);
	}
}
