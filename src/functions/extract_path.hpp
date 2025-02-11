#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the path from a URL or host
		std::string ExtractPath(const std::string &input);
	}
}
