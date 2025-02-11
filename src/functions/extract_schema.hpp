#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the schema from a URL
		std::string ExtractSchema(const std::string &input);
	}
}
