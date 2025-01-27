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

	// Function to extract the path from a URL or host
	std::string ExtractPath(const std::string &input);

	// Function to extract the host from a URL
	std::string ExtractHost(const std::string &input);

	// Function to extract the schema from a URL
	std::string ExtractSchema(const std::string &input);

	// Function to extract the query string from a URL
	std::string ExtractQueryString(const std::string &input);

	// Function to download the Tranco list and create a table
	void LoadTrancoList(DatabaseInstance &db, bool force);
} // namespace duckdb
