#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"

#include "utils/utils.hpp"
#include "functions/extract_domain.hpp"
#include "functions/extract_subdomain.hpp"
#include "functions/extract_tld.hpp"
#include "functions/extract_path.hpp"
#include "functions/extract_host.hpp"
#include "functions/extract_query.hpp"
#include "functions/extract_schema.hpp"
#include "functions/get_tranco.hpp"
#include "functions/get_version.hpp"

namespace duckdb
{
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
			netquack::UpdateSuffixesFunction);
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
			netquack::UpdateTrancoListFunction);
		ExtensionUtil::RegisterFunction(instance, netquack_update_tranco_function);

		auto get_tranco_rank_function = ScalarFunction(
			"get_tranco_rank",
			{LogicalType::VARCHAR},
			LogicalType::INTEGER,
			netquack::GetTrancoRankFunction);
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
}

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
