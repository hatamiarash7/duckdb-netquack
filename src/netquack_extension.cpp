// Copyright 2025 Arash Hatami

#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"

#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/parser/parsed_data/create_function_info.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "functions/base64_functions.hpp"
#include "functions/extract_domain.hpp"
#include "functions/domain_depth.hpp"
#include "functions/extract_extension.hpp"
#include "functions/extract_fragment.hpp"
#include "functions/extract_host.hpp"
#include "functions/extract_path.hpp"
#include "functions/extract_path_segments.hpp"
#include "functions/url_encode_functions.hpp"
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
#include "functions/parse_uri.hpp"
#include "functions/validation_functions.hpp"

namespace duckdb {

namespace {

FunctionDescription MakeDescription(vector<LogicalType> parameter_types, vector<string> parameter_names,
                                    string description, vector<string> examples, vector<string> categories) {
	FunctionDescription desc;
	desc.parameter_types = std::move(parameter_types);
	desc.parameter_names = std::move(parameter_names);
	desc.description = std::move(description);
	desc.examples = std::move(examples);
	desc.categories = std::move(categories);
	return desc;
}

void Register(ExtensionLoader &loader, ScalarFunction function, vector<string> parameter_names, string description,
              vector<string> examples, vector<string> categories) {
	auto types = function.arguments;
	CreateScalarFunctionInfo info(std::move(function));
	info.on_conflict = OnCreateConflict::ALTER_ON_CONFLICT;
	info.descriptions.push_back(MakeDescription(std::move(types), std::move(parameter_names), std::move(description),
	                                            std::move(examples), std::move(categories)));
	loader.RegisterFunction(std::move(info));
}

void Register(ExtensionLoader &loader, TableFunction function, vector<string> parameter_names, string description,
              vector<string> examples, vector<string> categories) {
	auto types = function.arguments;
	CreateTableFunctionInfo info(std::move(function));
	info.on_conflict = OnCreateConflict::ALTER_ON_CONFLICT;
	info.descriptions.push_back(MakeDescription(std::move(types), std::move(parameter_names), std::move(description),
	                                            std::move(examples), std::move(categories)));
	loader.RegisterFunction(std::move(info));
}

} // namespace

// Load the extension into the database
static void LoadInternal(ExtensionLoader &loader) {
	loader.SetDescription("Parsing, extracting, and analyzing domains, URIs, and paths with ease.");

	Register(loader,
	         ScalarFunction("extract_domain", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractDomainFunction),
	         {"url"}, "Extracts the registrable domain from a URL using the Public Suffix List.",
	         {"SELECT extract_domain('https://b.a.example.com/path');"}, {"url"});

	Register(loader, ScalarFunction("extract_path", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractPathFunction),
	         {"url"}, "Extracts the path component from a URL.",
	         {"SELECT extract_path('https://example.com/path/to/page');"}, {"url"});

	Register(loader,
	         ScalarFunction("extract_schema", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractSchemaFunction),
	         {"url"}, "Extracts the scheme (protocol) from a URL.",
	         {"SELECT extract_schema('https://example.com/path');"}, {"url"});

	Register(loader, ScalarFunction("extract_host", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractHostFunction),
	         {"url"}, "Extracts the host from a URL, excluding the port.",
	         {"SELECT extract_host('https://b.a.example.com/path');"}, {"url"});

	Register(loader,
	         ScalarFunction("extract_query_string", {LogicalType::VARCHAR}, LogicalType::VARCHAR,
	                        ExtractQueryStringFunction),
	         {"url"}, "Extracts the raw query string from a URL.",
	         {"SELECT extract_query_string('https://example.com/search?q=duckdb&hl=en');"}, {"url"});

	auto extract_query_parameters_function = TableFunction("extract_query_parameters", {LogicalType::VARCHAR}, nullptr,
	                                                       netquack::ExtractQueryParametersFunc::Bind, nullptr,
	                                                       netquack::ExtractQueryParametersFunc::InitLocal);
	extract_query_parameters_function.in_out_function = netquack::ExtractQueryParametersFunc::Function;
	Register(loader, std::move(extract_query_parameters_function), {"url"},
	         "Expands a URL query string into one row per key-value pair.",
	         {"SELECT * FROM extract_query_parameters('https://example.com/search?q=duckdb&hl=en');"}, {"url"});

	Register(loader, ScalarFunction("extract_tld", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractTLDFunction),
	         {"url"}, "Extracts the public suffix (TLD) from a URL, including multi-part suffixes.",
	         {"SELECT extract_tld('https://example.com.au/path');"}, {"url"});

	Register(
	    loader,
	    ScalarFunction("extract_subdomain", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractSubDomainFunction),
	    {"url"}, "Extracts the subdomain labels that precede the registrable domain.",
	    {"SELECT extract_subdomain('http://a.b.example.com/path');"}, {"url"});

	Register(loader, ScalarFunction("extract_port", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractPortFunction),
	         {"url"}, "Extracts the port from a URL.", {"SELECT extract_port('https://example.com:8443/');"}, {"url"});

	Register(
	    loader,
	    ScalarFunction("extract_extension", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractExtensionFunction),
	    {"url"}, "Extracts the file extension from a URL path, without the leading dot.",
	    {"SELECT extract_extension('http://example.com/image.jpg');"}, {"url"});

	Register(loader,
	         ScalarFunction("update_tranco", {LogicalType::BOOLEAN}, LogicalType::VARCHAR,
	                        netquack::UpdateTrancoListFunction),
	         {"force"}, "Downloads or refreshes the local Tranco top-sites list. Pass true to force a re-download.",
	         {"SELECT update_tranco(false);"}, {"tranco"});

	Register(loader,
	         ScalarFunction("get_tranco_rank", {LogicalType::VARCHAR}, LogicalType::VARCHAR,
	                        netquack::GetTrancoRankFunction),
	         {"domain"}, "Returns the Tranco rank of a domain.", {"SELECT get_tranco_rank('microsoft.com');"},
	         {"tranco"});

	Register(loader,
	         ScalarFunction("get_tranco_rank_category", {LogicalType::VARCHAR}, LogicalType::VARCHAR,
	                        netquack::GetTrancoRankCategoryFunction),
	         {"domain"}, "Returns the log-scale Tranco rank category of a domain (e.g. top1k, top10k).",
	         {"SELECT get_tranco_rank_category('microsoft.com');"}, {"tranco"});

	auto ipcalc_function = TableFunction("ipcalc", {LogicalType::VARCHAR}, nullptr, netquack::IPCalcFunc::Bind, nullptr,
	                                     netquack::IPCalcFunc::InitLocal);
	ipcalc_function.in_out_function = netquack::IPCalcFunc::Function;
	Register(loader, std::move(ipcalc_function), {"cidr"},
	         "Calculates network, broadcast, netmask, and host range for an IPv4 CIDR address.",
	         {"SELECT * FROM ipcalc('192.168.1.0/24');"}, {"ip"});

	Register(loader, ScalarFunction("is_valid_ip", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsValidIPFunction),
	         {"ip"}, "Returns true if the input is a valid IPv4 or IPv6 address.",
	         {"SELECT is_valid_ip('192.168.1.1');"}, {"ip"});

	Register(loader, ScalarFunction("is_private_ip", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsPrivateIPFunction),
	         {"ip"}, "Returns true if the IP belongs to a private or reserved range.",
	         {"SELECT is_private_ip('192.168.1.1');"}, {"ip"});

	Register(loader, ScalarFunction("ip_to_int", {LogicalType::VARCHAR}, LogicalType::UBIGINT, IPToIntFunction), {"ip"},
	         "Converts an IPv4 address to its 32-bit unsigned integer representation.",
	         {"SELECT ip_to_int('192.168.1.1');"}, {"ip"});

	Register(loader, ScalarFunction("int_to_ip", {LogicalType::UBIGINT}, LogicalType::VARCHAR, IntToIPFunction),
	         {"ip_int"}, "Converts a 32-bit unsigned integer back to an IPv4 address.",
	         {"SELECT int_to_ip(3232235777::UBIGINT);"}, {"ip"});

	Register(loader, ScalarFunction("ip_version", {LogicalType::VARCHAR}, LogicalType::TINYINT, IPVersionFunction),
	         {"ip"}, "Returns 4 for IPv4, 6 for IPv6, or NULL for invalid input.",
	         {"SELECT ip_version('192.168.1.1');"}, {"ip"});

	Register(loader,
	         ScalarFunction("ip_in_range", {LogicalType::VARCHAR, LogicalType::VARCHAR}, LogicalType::BOOLEAN,
	                        IPInRangeFunction),
	         {"ip", "cidr"}, "Returns true if the IP address falls within the given IPv4 or IPv6 CIDR block.",
	         {"SELECT ip_in_range('192.168.1.100', '192.168.1.0/24');"}, {"ip"});

	Register(loader, ScalarFunction("ip_to_ptr", {LogicalType::VARCHAR}, LogicalType::VARCHAR, IPToPTRFunction), {"ip"},
	         "Builds the reverse DNS (in-addr.arpa / ip6.arpa) name for an IPv4 or IPv6 address.",
	         {"SELECT ip_to_ptr('192.168.1.1');"}, {"ip"});

	Register(loader,
	         ScalarFunction("extract_fragment", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ExtractFragmentFunction),
	         {"url"}, "Extracts the fragment (the part after #) from a URL.",
	         {"SELECT extract_fragment('http://example.com/page#section');"}, {"url"});

	Register(loader, ScalarFunction("domain_depth", {LogicalType::VARCHAR}, LogicalType::INTEGER, DomainDepthFunction),
	         {"url"}, "Returns the number of dot-separated labels in a host or URL.",
	         {"SELECT domain_depth('https://www.example.com/page');"}, {"domain"});

	Register(loader,
	         ScalarFunction("normalize_url", {LogicalType::VARCHAR}, LogicalType::VARCHAR, NormalizeURLFunction),
	         {"url"},
	         "Canonicalizes a URL using RFC 3986 normalizations (scheme/host case, default ports, path, query order).",
	         {"SELECT normalize_url('HTTP://WWW.EXAMPLE.COM:80/a/b/../c/?z=1&a=2#frag');"}, {"url"});

	Register(loader,
	         ScalarFunction("base64_encode", {LogicalType::VARCHAR}, LogicalType::VARCHAR, Base64EncodeFunction),
	         {"string"}, "Encodes a string as Base64.", {"SELECT base64_encode('Hello World');"}, {"encoding"});

	Register(loader,
	         ScalarFunction("base64_decode", {LogicalType::VARCHAR}, LogicalType::VARCHAR, Base64DecodeFunction),
	         {"encoded"}, "Decodes a Base64 string back to its original form.",
	         {"SELECT base64_decode('SGVsbG8gV29ybGQ=');"}, {"encoding"});

	Register(loader, ScalarFunction("is_valid_url", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsValidURLFunction),
	         {"url"}, "Returns true if the input is a well-formed URL with a scheme and host.",
	         {"SELECT is_valid_url('https://example.com');"}, {"url"});

	Register(loader,
	         ScalarFunction("is_valid_domain", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, IsValidDomainFunction),
	         {"domain"}, "Returns true if the input is a valid domain name per RFC 1035/1123.",
	         {"SELECT is_valid_domain('example.com');"}, {"domain"});

	auto extract_path_segments_function =
	    TableFunction("extract_path_segments", {LogicalType::VARCHAR}, nullptr, netquack::ExtractPathSegmentsFunc::Bind,
	                  nullptr, netquack::ExtractPathSegmentsFunc::InitLocal);
	extract_path_segments_function.in_out_function = netquack::ExtractPathSegmentsFunc::Function;
	Register(loader, std::move(extract_path_segments_function), {"url"},
	         "Splits a URL path into one row per segment with a 1-based index.",
	         {"SELECT * FROM extract_path_segments('https://example.com/path/to/page');"}, {"url"});

	auto parse_uri_type = LogicalType::STRUCT({{"scheme", LogicalType(LogicalTypeId::VARCHAR)},
	                                           {"host", LogicalType(LogicalTypeId::VARCHAR)},
	                                           {"port", LogicalType(LogicalTypeId::VARCHAR)},
	                                           {"path", LogicalType(LogicalTypeId::VARCHAR)},
	                                           {"query", LogicalType(LogicalTypeId::VARCHAR)},
	                                           {"fragment", LogicalType(LogicalTypeId::VARCHAR)}});
	Register(loader, ScalarFunction("parse_uri", {LogicalType::VARCHAR}, std::move(parse_uri_type), ParseURIFunction),
	         {"url"}, "Parses a URI and returns a STRUCT with scheme, host, port, path, query, and fragment.",
	         {"SELECT parse_uri('https://example.com:8080/path?q=1#section');"}, {"url"});

	Register(loader, ScalarFunction("url_encode", {LogicalType::VARCHAR}, LogicalType::VARCHAR, UrlEncodeFunction),
	         {"string"}, "Percent-encodes a string per RFC 3986.", {"SELECT url_encode('hello world');"}, {"encoding"});

	Register(loader, ScalarFunction("url_decode", {LogicalType::VARCHAR}, LogicalType::VARCHAR, UrlDecodeFunction),
	         {"encoded"}, "Decodes a percent-encoded string. Also treats '+' as a space.",
	         {"SELECT url_decode('hello%20world');"}, {"encoding"});

	Register(loader,
	         TableFunction("netquack_version", {}, netquack::VersionFunc::Scan, netquack::VersionFunc::Bind,
	                       netquack::VersionFunc::InitGlobal, netquack::VersionFunc::InitLocal),
	         {}, "Returns the installed netquack extension version.", {"SELECT * FROM netquack_version();"},
	         {"utility"});
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
