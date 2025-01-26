#pragma once

#include "duckdb.hpp"

namespace duckdb
{

	// Function to download a file from a URL
	size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

	// Function to download the public suffix list
	std::string DownloadPublicSuffixList();

	// Function to parse the public suffix list and store it in a table
	void LoadPublicSuffixList(DatabaseInstance &db, bool force);

} // namespace duckdb
