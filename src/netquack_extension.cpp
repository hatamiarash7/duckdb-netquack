#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"

#include "utils/utils.hpp"
#include "functions/get_version.hpp"
#include "functions/extract_domain.hpp"
#include "functions/extract_subdomain.hpp"
#include "functions/extract_tld.hpp"
#include "functions/extract_path.hpp"
#include "functions/extract_host.hpp"
#include "functions/extract_query.hpp"
#include "functions/extract_schema.hpp"

#include <regex>

namespace duckdb
{
	// Function to update the public suffix list table
	static void UpdateSuffixesFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Load the public suffix list if not already loaded
		auto &db = *state.GetContext().db;
		netquack::LoadPublicSuffixList(db, true);

		result.SetValue(0, Value("updated"));
	}

	static void ExtractDomainFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the domain using the utility function
		auto domain = netquack::ExtractDomain(state, input);

		result.SetValue(0, Value(domain));
	}

	// Function to extract the path from a URL
	static void ExtractPathFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the path using the utility function
		auto path = netquack::ExtractPath(input);

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
		auto host = netquack::ExtractHost(input);

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
		auto schema = netquack::ExtractSchema(input);

		// Set the result
		result.SetValue(0, Value(schema));
	}

	static void ExtractQueryStringFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the URL from the input
		auto &url_vector = args.data[0];
		auto url = url_vector.GetValue(0).ToString();

		// Extract the query string
		auto query_string = netquack::ExtractQueryString(url);

		// Set the result
		result.SetValue(0, Value(query_string));
	}

	// Function to extract the top-level domain from a URL
	static void ExtractTLDFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the top-level domain using the utility function
		auto tld = netquack::ExtractTLD(state, input);

		// Set the result
		result.SetValue(0, Value(tld));
	}

	// Function to extract the sub-domain from a URL
	static void ExtractSubDomainFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		// Extract the input from the arguments
		auto &input_vector = args.data[0];
		auto input = input_vector.GetValue(0).ToString();

		// Extract the sub-domain using the utility function
		auto subdomain = netquack::ExtractSubDomain(state, input);

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
		netquack::LoadTrancoList(db, force_download);

		result.SetValue(0, Value("Tranco list updated"));
	}

	// Function to get the Tranco rank of a domain
	static void GetTrancoRankFunction(DataChunk &args, ExpressionState &state, Vector &result)
	{
		auto &db = *state.GetContext().db;
		netquack::LoadPublicSuffixList(db, false);
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

		auto version_function = TableFunction(
			"netquack_version",
			{},
			netquack::VersionFunc::Scan,
			netquack::VersionFunc::Bind,
			netquack::VersionFunc::InitGlobal,
			netquack::VersionFunc::InitLocal);
		ExtensionUtil::RegisterFunction(instance, version_function);
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
