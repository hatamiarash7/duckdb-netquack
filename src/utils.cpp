#include "utils.hpp"
#include <curl/curl.h>
#include <sstream>
#include <regex>
#include <fstream>

namespace duckdb
{
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
}
