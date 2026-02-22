// Copyright 2025 Arash Hatami

#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"

#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "functions/base64_functions.hpp"
#include "functions/extract_domain.hpp"
#include "functions/domain_depth.hpp"
#include "functions/extract_extension.hpp"
#include "functions/extract_fragment.hpp"
#include "functions/extract_host.hpp"
#include "functions/extract_path.hpp"
#include "functions/extract_path_segments.hpp"
#include "functions/extract_port.hpp"
#include "functions/extract_query.hpp"
#include "functions/extract_schema.hpp"
#include "functions/extract_subdomain.hpp"
#include "functions/extract_tld.hpp"
#include "functions/get_tranco.hpp"
#include "functions/get_version.hpp"
#include "functions/ip_functions.hpp"
#include "functions/ipcalc.hpp"
#include "functions/normalize_url.hpp"
#include "functions/validation_functions.hpp"

namespace duckdb {
// Load the extension into the database
static void LoadInternal(ExtensionLoader &loader) {
	loader.SetDescription("Parsing, extracting, and analyzing domains, URIs, and paths with ease.");

	auto netquack_extract_domain_function =
	    ScalarFunction("extract_domain", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractDomainFunction);
	loader.RegisterFunction(netquack_extract_domain_function);

	auto netquack_extract_path_function =
	    ScalarFunction("extract_path", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractPathFunction);
	loader.RegisterFunction(netquack_extract_path_function);

	auto netquack_extract_schema_function =
	    ScalarFunction("extract_schema", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractSchemaFunction);
	loader.RegisterFunction(netquack_extract_schema_function);

	auto netquack_extract_host_function =
	    ScalarFunction("extract_host", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractHostFunction);
	loader.RegisterFunction(netquack_extract_host_function);

	auto netquack_extract_query_string_function = ScalarFunction("extract_query_string", {LogicalType::VARCHAR},
	                                                             LogicalType::VARCHAR, ExtractQueryStringFunction);
	loader.RegisterFunction(netquack_extract_query_string_function);

	auto extract_query_parameters_function = TableFunction("extract_query_parameters", {LogicalType::VARCHAR}, nullptr,
	                                                       netquack::ExtractQueryParametersFunc::Bind, nullptr,
	                                                       netquack::ExtractQueryParametersFunc::InitLocal);
	extract_query_parameters_function.in_out_function = netquack::ExtractQueryParametersFunc::Function;
	loader.RegisterFunction(extract_query_parameters_function);

	auto netquack_extract_tld_function =
	    ScalarFunction("extract_tld", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractTLDFunction);
	loader.RegisterFunction(netquack_extract_tld_function);

	auto netquack_extract_subdomain_function =
	    ScalarFunction("extract_subdomain", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractSubDomainFunction);
	loader.RegisterFunction(netquack_extract_subdomain_function);

	auto netquack_extract_port_function =
	    ScalarFunction("extract_port", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractPortFunction);
	loader.RegisterFunction(netquack_extract_port_function);

	auto netquack_extract_extension_function =
	    ScalarFunction("extract_extension", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractExtensionFunction);
	loader.RegisterFunction(netquack_extract_extension_function);

	auto netquack_update_tranco_function = ScalarFunction("update_tranco", {LogicalType::BOOLEAN}, LogicalType::VARCHAR,
	                                                      netquack::UpdateTrancoListFunction);
	loader.RegisterFunction(netquack_update_tranco_function);

	auto get_tranco_rank_function = ScalarFunction("get_tranco_rank", {LogicalType::VARCHAR}, LogicalType::VARCHAR,
	                                               netquack::GetTrancoRankFunction);
	loader.RegisterFunction(get_tranco_rank_function);

	auto get_tranco_rank_category_function =
	    ScalarFunction("get_tranco_rank_category", {LogicalType::VARCHAR}, LogicalType::VARCHAR,
	                   netquack::GetTrancoRankCategoryFunction);
	loader.RegisterFunction(get_tranco_rank_category_function);

	auto ipcalc_function = TableFunction("ipcalc", {LogicalType::VARCHAR}, nullptr, netquack::IPCalcFunc::Bind, nullptr,
	                                     netquack::IPCalcFunc::InitLocal);
	ipcalc_function.in_out_function = netquack::IPCalcFunc::Function;
	loader.RegisterFunction(ipcalc_function);

	auto is_valid_ip_function =
	    ScalarFunction("is_valid_ip", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsValidIPFunction);
	loader.RegisterFunction(is_valid_ip_function);

	auto is_private_ip_function =
	    ScalarFunction("is_private_ip", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsPrivateIPFunction);
	loader.RegisterFunction(is_private_ip_function);

	auto ip_to_int_function =
	    ScalarFunction("ip_to_int", {LogicalType::VARCHAR}, LogicalType::UBIGINT, IPToIntFunction);
	loader.RegisterFunction(ip_to_int_function);

	auto int_to_ip_function =
	    ScalarFunction("int_to_ip", {LogicalType::UBIGINT}, LogicalType::VARCHAR, IntToIPFunction);
	loader.RegisterFunction(int_to_ip_function);

	auto ip_version_function =
	    ScalarFunction("ip_version", {LogicalType::VARCHAR}, LogicalType::TINYINT, IPVersionFunction);
	loader.RegisterFunction(ip_version_function);

	auto netquack_extract_fragment_function =
	    ScalarFunction("extract_fragment", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractFragmentFunction);
	loader.RegisterFunction(netquack_extract_fragment_function);

	auto domain_depth_function =
	    ScalarFunction("domain_depth", {LogicalType::VARCHAR}, LogicalType::INTEGER, DomainDepthFunction);
	loader.RegisterFunction(domain_depth_function);

	auto normalize_url_function =
	    ScalarFunction("normalize_url", {LogicalType::VARCHAR}, LogicalType::VARCHAR, NormalizeURLFunction);
	loader.RegisterFunction(normalize_url_function);

	auto base64_encode_function =
	    ScalarFunction("base64_encode", {LogicalType::VARCHAR}, LogicalType::VARCHAR, Base64EncodeFunction);
	loader.RegisterFunction(base64_encode_function);

	auto base64_decode_function =
	    ScalarFunction("base64_decode", {LogicalType::VARCHAR}, LogicalType::VARCHAR, Base64DecodeFunction);
	loader.RegisterFunction(base64_decode_function);

	auto is_valid_url_function =
	    ScalarFunction("is_valid_url", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsValidURLFunction);
	loader.RegisterFunction(is_valid_url_function);

	auto is_valid_domain_function =
	    ScalarFunction("is_valid_domain", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsValidDomainFunction);
	loader.RegisterFunction(is_valid_domain_function);

	auto extract_path_segments_function =
	    TableFunction("extract_path_segments", {LogicalType::VARCHAR}, nullptr, netquack::ExtractPathSegmentsFunc::Bind,
	                  nullptr, netquack::ExtractPathSegmentsFunc::InitLocal);
	extract_path_segments_function.in_out_function = netquack::ExtractPathSegmentsFunc::Function;
	loader.RegisterFunction(extract_path_segments_function);

	auto version_function =
	    TableFunction("netquack_version", {}, netquack::VersionFunc::Scan, netquack::VersionFunc::Bind,
	                  netquack::VersionFunc::InitGlobal, netquack::VersionFunc::InitLocal);
	loader.RegisterFunction(version_function);
}

void NetquackExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
}
std::string NetquackExtension::Name() {
	return "netquack";
}

std::string NetquackExtension::Version() const {
#ifdef EXT_VERSION_NETQUACK
	return EXT_VERSION_NETQUACK;
#else
	return "";
#endif
}
} // namespace duckdb

extern "C" {
DUCKDB_CPP_EXTENSION_ENTRY(netquack, loader) {
	duckdb::LoadInternal(loader);
}
}
