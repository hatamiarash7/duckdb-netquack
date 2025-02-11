#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to extract the host from a URL
		std::string ExtractHost(const std::string &input);
	}
}
