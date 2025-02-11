#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	// Function to extract the top-level domain from a URL
	void ExtractTLDFunction(DataChunk &args, ExpressionState &state, Vector &result);

	namespace netquack
	{
		// Function to extract the top-level domain from a URL
		std::string ExtractTLD(ExpressionState &state, const std::string &input);
	}
}
