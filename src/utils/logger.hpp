#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    namespace netquack
    {
        enum class LogLevel
        {
            DEBUG,
            INFO,
            WARNING,
            ERROR,
            CRITICAL
        };

        // Function to log messages with a specified log level
        void LogMessage (LogLevel level, const std::string &message);
    } // namespace netquack
} // namespace duckdb
