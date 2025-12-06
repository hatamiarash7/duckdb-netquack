// Copyright 2025 Arash Hatami

#include "logger.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace duckdb
{
    namespace netquack
    {
        // Thread-safe mutex for logging
        static std::mutex log_mutex;

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
            std::tm tm_buf{};
#ifdef _WIN32
            localtime_s (&tm_buf, &now);
#else
            localtime_r (&now, &tm_buf);
#endif
            std::ostringstream oss;
            oss << std::put_time (&tm_buf, "%Y-%m-%d %H:%M:%S");
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

            // Lock for thread safety
            std::lock_guard<std::mutex> lock (log_mutex);

            std::ofstream log_file ("netquack.log", std::ios_base::app);
            if (log_file.is_open ())
            {
                log_file << "[" << getCurrentTimestamp () << "] "
                         << level_str << " - "
                         << message << std::endl;
            }

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
