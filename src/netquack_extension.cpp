#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>
#include <regex>
#include <fstream>
#include <sstream>
#include <curl/curl.h>

namespace duckdb
{

	// Function to download a file from a URL
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((std::string *)userp)->append((char *)contents, size * nmemb);
		return size * nmemb;
	}

	static std::string DownloadPublicSuffixList()
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
	static void LoadPublicSuffixList(DatabaseInstance &db, bool force = false)
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

	static void UpdateSuffixesFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Load the public suffix list if not already loaded
		auto &db = *state.GetContext().db;
		LoadPublicSuffixList(db, true);

		result.SetValue(0, Value("updated"));
	}

	// Function to extract the main domain from a URL
	static void ExtractDomainFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Load the public suffix list if not already loaded
		auto &db = *state.GetContext().db;
		LoadPublicSuffixList(db);
		Connection con(db);

		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto result_data = FlatVector::GetData<string_t>(result);

		for (idx_t i = 0; i < args.size(); i++)
		{
			auto url = url_vector.GetValue(i).ToString();

			// Extract the host from the URL
			std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\?:]+))");
			std::smatch host_match;
			if (!std::regex_search(url, host_match, host_regex))
			{
				result_data[i] = StringVector::AddString(result, "");
				continue;
			}

			auto host = host_match[1].str();

			// Split the host into parts
			std::vector<std::string> parts;
			std::istringstream stream(host);
			std::string part;
			while (std::getline(stream, part, '.'))
			{
				parts.push_back(part);
			}

			// Find the longest matching public suffix
			std::string public_suffix;
			int public_suffix_index = -1;

			for (int j = 0; j < parts.size(); j++)
			{
				// Build the candidate suffix
				std::string candidate;
				for (int k = j; k < parts.size(); k++)
				{
					candidate += (k == j ? "" : ".") + parts[k];
				}

				// Query the public suffix list
				auto query = "SELECT 1 FROM public_suffix_list WHERE suffix = '" + candidate + "'";
				auto query_result = con.Query(query);

				if (query_result->RowCount() > 0)
				{
					public_suffix = candidate;
					public_suffix_index = j;
					break;
				}
			}

			// Determine the main domain
			std::string domain;
			if (!public_suffix.empty() && public_suffix_index > 0)
			{
				// Combine the part before the public suffix with the public suffix
				domain = parts[public_suffix_index - 1] + "." + public_suffix;
			}
			else if (!public_suffix.empty())
			{
				// No part before the suffix, use the public suffix only
				domain = public_suffix;
			}

			result_data[i] = StringVector::AddString(result, domain);
		}
	}

	static void LoadInternal(DatabaseInstance &instance)
	{
		auto netquack_extract_domain_function = ScalarFunction(
			"extract_domain",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractDomainFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_domain_function);

		auto netquack_update_suffixes_function = ScalarFunction(
			"update_suffixes",
			{},
			LogicalType::VARCHAR,
			UpdateSuffixesFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_update_suffixes_function);
	}

	void NetquackExtension::Load(DuckDB &db)
	{
		LoadInternal(*db.instance);
	}
	std::string NetquackExtension::Name()
	{
		return "netquack";
	}

	std::string NetquackExtension::Version() const
	{
#ifdef EXT_VERSION_NETQUACK
		return EXT_VERSION_NETQUACK;
#else
		return "";
#endif
	}

} // namespace duckdb

extern "C"
{

	DUCKDB_EXTENSION_API void netquack_init(duckdb::DatabaseInstance &db)
	{
		duckdb::DuckDB db_wrapper(db);
		db_wrapper.LoadExtension<duckdb::NetquackExtension>();
	}

	DUCKDB_EXTENSION_API const char *netquack_version()
	{
		return duckdb::DuckDB::LibraryVersion();
	}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
