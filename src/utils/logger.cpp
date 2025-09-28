// Copyright 2025 Arash Hatami

#include "logger.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>

namespace duckdb
{
    namespace netquack
    {
        LogLevel getLogLevelFromEnv ()
        {
            const char *env_level = std::getenv ("LOG_LEVEL");
            if (env_level == nullptr)
            {
                return LogLevel::LOG_INFO; // default level
            }

            std::string level_str (env_level);
            if (level_str == "DEBUG")
                return LogLevel::LOG_DEBUG;
            if (level_str == "INFO")
                return LogLevel::LOG_INFO;
            if (level_str == "WARNING")
                return LogLevel::LOG_WARNING;
            if (level_str == "ERROR")
                return LogLevel::LOG_ERROR;
            if (level_str == "CRITICAL")
                return LogLevel::LOG_CRITICAL;

            std::cerr << "Unknown LOG_LEVEL environment variable value: " << level_str
                      << ". Defaulting to INFO." << std::endl;
            return LogLevel::LOG_INFO;
        }

        std::string getCurrentTimestamp ()
        {
            auto now = std::time (nullptr);
            auto tm  = *std::localtime (&now);
            std::ostringstream oss;
            oss << std::put_time (&tm, "%Y-%m-%d %H:%M:%S");
            return oss.str ();
        }

        void LogMessage (LogLevel level, const std::string &message)
        {
            static const LogLevel min_log_level = getLogLevelFromEnv ();

            // Skip if message level is below minimum configured level
            if (level < min_log_level)
            {
                return;
            }

            const char *level_str = "";
            switch (level)
            {
            case LogLevel::LOG_DEBUG: level_str = "DEBUG"; break;
            case LogLevel::LOG_INFO: level_str = "INFO"; break;
            case LogLevel::LOG_WARNING: level_str = "WARNING"; break;
            case LogLevel::LOG_ERROR: level_str = "ERROR"; break;
            case LogLevel::LOG_CRITICAL: level_str = "CRITICAL"; break;
            }

            std::ofstream log_file ("netquack.log", std::ios_base::app);
            if (!log_file.is_open ())
            {
                std::cerr << "Failed to open log file!" << std::endl;
                return;
            }

            log_file << "[" << getCurrentTimestamp () << "] "
                     << level_str << " - "
                     << message << std::endl;

            // Also output to stderr for error levels
            if (level >= LogLevel::LOG_ERROR)
            {
                std::cerr << "[" << level_str << "] " << message << std::endl;
            }

            // Throw exception for critical errors
            if (level == LogLevel::LOG_CRITICAL)
            {
                throw std::runtime_error (message);
            }
        }
    } // namespace netquack
} // namespace duckdb
