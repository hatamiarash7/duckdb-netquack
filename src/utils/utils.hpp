#pragma once

#include "duckdb.hpp"

namespace duckdb
{
	namespace netquack
	{
		// Function to download a file from a URL
		size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

		// Function to download the public suffix list
		std::string DownloadPublicSuffixList();

		// Function to parse the public suffix list and store it in a table
		void LoadPublicSuffixList(DatabaseInstance &db, bool force);

		// Function to update the public suffix list table
		void UpdateSuffixesFunction(DataChunk &args, ExpressionState &state, Vector &result);

		// Function to download the Tranco list and create a table
		void LoadTrancoList(DatabaseInstance &db, bool force);
	}
}
