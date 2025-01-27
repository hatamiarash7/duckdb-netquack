#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "utils.hpp"

#include <regex>

namespace duckdb
{
	// Function to update the public suffix list table
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
		LoadPublicSuffixList(db, false);
		Connection con(db);

		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto url = url_vector.GetValue(0).ToString();

		// Extract the host from the URL
		std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\?:]+))");
		std::smatch host_match;
		if (!std::regex_search(url, host_match, host_regex))
		{
			result.SetValue(0, Value(""));
			return;
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

		result.SetValue(0, Value(domain));
	}

	// Function to extract the path from a URL
	static void ExtractPathFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the path using the utility function
		auto path = ExtractPath(input);

		// Set the result
		result.SetValue(0, Value(path));
	}

	// Function to extract the host from a URL
	static void ExtractHostFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the host using the utility function
		auto host = ExtractHost(input);

		// Set the result
		result.SetValue(0, Value(host));
	}

	// Function to extract the schema from a URL
	static void ExtractSchemaFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the schema using the utility function
		auto schema = ExtractSchema(input);

		// Set the result
		result.SetValue(0, Value(schema));
	}

	static void ExtractQueryStringFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto url = url_vector.GetValue(0).ToString();

		// Extract the query string
		auto query_string = ExtractQueryString(url);

		// Set the result
		result.SetValue(0, Value(query_string));
	}

	// Function to extract the top-level domain from a URL
	static void ExtractTLDFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Load the public suffix list if not already loaded
		auto &db = *state.GetContext().db;
		LoadPublicSuffixList(db, false);
		Connection con(db);

		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto url = url_vector.GetValue(0).ToString();

		// Extract the host from the URL
		std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\?:]+))");
		std::smatch host_match;
		if (!std::regex_search(url, host_match, host_regex))
		{
			result.SetValue(0, Value(""));
			return;
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

		result.SetValue(0, Value(public_suffix));
	}

	// Function to extract the sub-domain from a URL
	static void ExtractSubDomainFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Load the public suffix list if not already loaded
		auto &db = *state.GetContext().db;
		LoadPublicSuffixList(db, false);
		Connection con(db);

		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto url = url_vector.GetValue(0).ToString();

		// Extract the host from the URL
		std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\?:]+))");
		std::smatch host_match;
		if (!std::regex_search(url, host_match, host_regex))
		{
			result.SetValue(0, Value(""));
			return;
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

		// Determine the subdomain
		std::string subdomain;
		if (!public_suffix.empty() && public_suffix_index > 0)
		{
			// Combine all parts before the public suffix
			for (int i = 0; i < public_suffix_index - 1; i++)
			{
				subdomain += (i == 0 ? "" : ".") + parts[i];
			}
		}

		result.SetValue(0, Value(subdomain));
	}

	// Function to update the Tranco list table
	static void UpdateTrancoListFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the force_download argument
		auto &force_download_vector = args.data[0];
		bool force_download = force_download_vector.GetValue(0).GetValue<bool>();

		// Load the Tranco list into the database
		auto &db = *state.GetContext().db;
		LoadTrancoList(db, force_download);

		result.SetValue(0, Value("Tranco list updated"));
	}

	// Function to get the Tranco rank of a domain
	static void GetTrancoRankFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		auto &db = *state.GetContext().db;
		LoadPublicSuffixList(db, false);
		Connection con(db);

		auto table_exists = con.Query("SELECT 1 FROM information_schema.tables WHERE table_name = 'tranco_list'");

		if (table_exists->RowCount() == 0)
		{
			throw std::runtime_error("Tranco table not found. Download it first using `SELECT update_tranco(true);`");
		}

		auto &domain_vector = args.data[0];
		auto domain = domain_vector.GetValue(0).ToString();

		auto query = "SELECT rank FROM tranco_list WHERE domain = '" + domain + "'";
		auto query_result = con.Query(query);

		result.SetValue(0, query_result->RowCount() > 0 ? query_result->GetValue(0, 0) : Value());
	}

	// Load the extension into the database
	static void LoadInternal(DatabaseInstance &instance)
	{
		ExtensionUtil::RegisterExtension(
			instance,
			"netquack",
			{"Parsing, extracting, and analyzing domains, URIs, and paths with ease."});

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

		auto netquack_extract_path_function = ScalarFunction(
			"extract_path",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractPathFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_path_function);

		auto netquack_extract_schema_function = ScalarFunction(
			"extract_schema",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractSchemaFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_schema_function);

		auto netquack_extract_host_function = ScalarFunction(
			"extract_host",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractHostFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_host_function);

		auto netquack_extract_query_string_function = ScalarFunction(
			"extract_query_string",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractQueryStringFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_query_string_function);

		auto netquack_extract_tld_function = ScalarFunction(
			"extract_tld",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractTLDFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_tld_function);

		auto netquack_extract_subdomain_function = ScalarFunction(
			"extract_subdomain",
			{LogicalType::VARCHAR},
			LogicalType::VARCHAR,
			ExtractSubDomainFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_extract_subdomain_function);

		auto netquack_update_tranco_function = ScalarFunction(
			"update_tranco",
			{LogicalType::BOOLEAN},
			LogicalType::VARCHAR,
			UpdateTrancoListFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_update_tranco_function);

		auto get_tranco_rank_function = ScalarFunction(
			"get_tranco_rank",
			{LogicalType::VARCHAR},
			LogicalType::INTEGER,
			GetTrancoRankFunction);
		ExtensionUtil::RegisterFunction(instance, get_tranco_rank_function);
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
