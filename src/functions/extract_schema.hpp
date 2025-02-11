#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	// Function to extract the schema from a URL
	void ExtractSchemaFunction(DataChunk &args, ExpressionState &state, Vector &result);

	namespace netquack
	{
		// Function to extract the schema from a URL
		std::string ExtractSchema(const std::string &input);
	}
}
