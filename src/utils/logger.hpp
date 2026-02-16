// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
namespace netquack {
// Note: `LOG_` prefix is to avoid problems with DEBUG and ERROR macros
enum class LogLevel { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_CRITICAL };

// Function to log messages with a specified log level
void LogMessage(LogLevel level, const std::string &message);
} // namespace netquack
} // namespace duckdb
