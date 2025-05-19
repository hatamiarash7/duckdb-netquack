#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    namespace netquack
    {
        // Function to log messages with a specified log level
        void LogMessage (const std::string &level, const std::string &message);
    } // namespace netquack
} // namespace duckdb
