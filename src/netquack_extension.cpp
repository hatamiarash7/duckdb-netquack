#define DUCKDB_EXTENSION_MAIN

#include "netquack_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

// OpenSSL linked through vcpkg
#include <openssl/opensslv.h>

namespace duckdb {

inline void NetquackScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(
	    name_vector, result, args.size(),
	    [&](string_t name) {
			return StringVector::AddString(result, "Netquack "+name.GetString()+" 🐥");;
        });
}

inline void NetquackOpenSSLVersionScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(
	    name_vector, result, args.size(),
	    [&](string_t name) {
			return StringVector::AddString(result, "Netquack " + name.GetString() +
                                                     ", my linked OpenSSL version is " +
                                                     OPENSSL_VERSION_TEXT );;
        });
}

static void LoadInternal(DatabaseInstance &instance) {
    // Register a scalar function
    auto netquack_scalar_function = ScalarFunction("netquack", {LogicalType::VARCHAR}, LogicalType::VARCHAR, NetquackScalarFun);
    ExtensionUtil::RegisterFunction(instance, netquack_scalar_function);

    // Register another scalar function
    auto netquack_openssl_version_scalar_function = ScalarFunction("netquack_openssl_version", {LogicalType::VARCHAR},
                                                LogicalType::VARCHAR, NetquackOpenSSLVersionScalarFun);
    ExtensionUtil::RegisterFunction(instance, netquack_openssl_version_scalar_function);
}

void NetquackExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
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

DUCKDB_EXTENSION_API void netquack_init(duckdb::DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::NetquackExtension>();
}

DUCKDB_EXTENSION_API const char *netquack_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
