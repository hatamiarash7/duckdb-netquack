#include "utils.hpp"
#include <curl/curl.h>
#include <regex>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <fstream>

namespace duckdb
{
	void LogMessage(const std::string &level, const std::string &message)
	{
		std::ofstream log_file("netquack.log", std::ios_base::app);
		log_file << "[" << level << "] " << message << std::endl;

		if (level == "ERROR")
		{
			throw std::runtime_error(message);
		}
	}

	size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((std::string *)userp)->append((char *)contents, size * nmemb);
		return size * nmemb;
	}

	std::string DownloadPublicSuffixList()
	{
		CURL *curl;
		CURLcode res;
		std::string readBuffer;

		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, "https://publicsuffix.org/list/public_suffix_list.dat");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (res != CURLE_OK)
			{
				throw std::runtime_error("Failed to download public suffix list.");
			}
		}

		return readBuffer;
	}

	// Function to parse the public suffix list and store it in a table
	void LoadPublicSuffixList(DatabaseInstance &db, bool force)
	{
		// Check if the table already exists
		Connection con(db);
		auto table_exists = con.Query("SELECT 1 FROM information_schema.tables WHERE table_name = 'public_suffix_list'");

		if (table_exists->RowCount() == 0 || force)
		{
			// Download the list
			auto list_data = DownloadPublicSuffixList();

			// Parse the list and insert into a table
			std::istringstream stream(list_data);
			std::string line;
			con.Query("CREATE OR REPLACE TABLE public_suffix_list (suffix VARCHAR)");

			while (std::getline(stream, line))
			{
				// Skip comments and empty lines
				if (line.empty() || line[0] == '/' || line[0] == ' ')
					continue;

				// Replace `*.` with an empty string
				size_t wildcard_pos = line.find("*.");
				if (wildcard_pos != std::string::npos)
				{
					line.replace(wildcard_pos, 2, "");
				}

				// Insert the suffix into the table
				con.Query("INSERT INTO public_suffix_list (suffix) VALUES ('" + line + "')");
			}
		}
	}

	// Function to extract the path from a URL or host
	std::string ExtractPath(const std::string &input)
	{
		// Regex to match the path component
		std::regex path_regex(R"(^(?:(?:https?:\/\/)?(?:[^\/\s]+))(\/[^?#]*))");
		std::smatch path_match;

		if (std::regex_match(input, path_match, path_regex))
		{
			// If a path is found, return it
			if (path_match.size() > 1 && path_match[1].matched)
			{
				return path_match[1].str();
			}
		}

		// If no path is found, return "/"
		return "/";
	}

	// Function to extract the host from a URL
	std::string ExtractHost(const std::string &input)
	{
		// Regex to match the host component
		std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\s:?#]+))");
		std::smatch host_match;

		if (std::regex_search(input, host_match, host_regex))
		{
			// If a host is found, return it
			if (host_match.size() > 1 && host_match[1].matched)
			{
				return host_match[1].str();
			}
		}

		// If no host is found, return ""
		return "'";
	}

	// Function to get the download code for the Tranco list
	std::string GetTrancoDownloadCode(char *date)
	{
		CURL *curl;
		CURLcode res;
		std::string readBuffer;

		// Construct the URL for the daily list
		std::string url = "https://tranco-list.eu/daily_list?date=" + std::string(date) + "&subdomains=true";

		LogMessage("INFO", "Get Tranco download code for date: " + std::string(date));

		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (res != CURLE_OK)
			{
				throw std::runtime_error("Failed to fetch Tranco download code.");
			}
		}

		// Extract the download code from the URL
		std::regex code_regex(R"(Information on the Tranco list with ID ([A-Z0-9]+))");
		std::smatch code_match;
		if (std::regex_search(readBuffer, code_match, code_regex) && code_match.size() > 1)
		{
			LogMessage("INFO", "Tranco download code: " + code_match[1].str());
			return code_match[1].str();
		}

		throw std::runtime_error("Failed to extract Tranco download code.");
	}

	// Function to download the Tranco list and create a table
	void LoadTrancoList(DatabaseInstance &db, bool force)
	{
		// Get yesterday's date in YYYY-MM-DD format
		std::time_t now = std::time(nullptr);
		std::tm *yesterday = std::localtime(&now);
		yesterday->tm_mday -= 1; // Subtract one day
		std::mktime(yesterday);	 // Normalize the time
		char date[11];
		std::strftime(date, sizeof(date), "%Y-%m-%d", yesterday);

		// Construct the file name
		std::string temp_file = "tranco_list_" + std::string(date) + ".csv";

		// Download the file if it doesn't exist or if force is true
		std::ifstream file(temp_file);
		if (force)
		{
			// Remove the old file if it exists
			if (file.good())
			{
				remove(temp_file.c_str());
			}
			// Get the download code
			std::string download_code = GetTrancoDownloadCode(date);

			// Construct the download URL
			std::string download_url = "https://tranco-list.eu/download/" + download_code + "/full";

			LogMessage("INFO", "Download Tranco list: " + download_url);

			// Download the CSV file to a temporary file
			CURL *curl;
			CURLcode res;
			FILE *file = fopen(temp_file.c_str(), "wb");
			if (!file)
			{
				throw std::runtime_error("Failed to create temporary file for Tranco list.");
			}

			curl = curl_easy_init();
			if (curl)
			{
				curl_easy_setopt(curl, CURLOPT_URL, download_url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
				res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);
				fclose(file);

				if (res != CURLE_OK)
				{
					remove(temp_file.c_str()); // Clean up the temporary file
					throw std::runtime_error("Failed to download Tranco list.");
				}
			}
		}

		if (!file.good())
		{
			LogMessage("ERROR", "Tranco list not found. Download it first using `SELECT update_tranco(true);`");
		}

		// Parse the CSV data and insert into a table
		LogMessage("INFO", "Inserting Tranco list into table");

		Connection con(db);
		con.Query("CREATE OR REPLACE TABLE tranco_list AS SELECT * FROM read_csv('" + temp_file + "', header=false, columns={'rank': 'INTEGER', 'domain': 'VARCHAR'})");
	}
}
