// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
void ParseURIFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
struct ParsedURI {
	std::string scheme;
	std::string host;
	std::string port;
	std::string path = "/";
	std::string query;
	std::string fragment;
};

ParsedURI ParseURI(const std::string_view &input);
} // namespace netquack
} // namespace duckdb
