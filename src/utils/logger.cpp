// Copyright 2025 Arash Hatami

#include "logger.hpp"

#include <fstream>
#include <iomanip> // For std::put_time
#include <iostream>
#include <mutex> // For std::mutex
#include <sstream> // For std::ostringstream
#include <ctime>   // For std::time_t, std::tm

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // For POSIX version if needed for other things
#endif

namespace duckdb
{
    namespace netquack
    {
        LogLevel getLogLevelFromEnv ()
        {
            const char *env_level = std::getenv ("LOG_LEVEL");
            if (env_level == nullptr)
            {
                return LogLevel::INFO; // default level
            }

            std::string level_str (env_level);
            if (level_str == "DEBUG")
                return LogLevel::DEBUG;
            if (level_str == "INFO")
                return LogLevel::INFO;
            if (level_str == "WARNING")
                return LogLevel::WARNING;
            if (level_str == "ERROR")
                return LogLevel::ERROR;
            if (level_str == "CRITICAL")
                return LogLevel::CRITICAL;

            std::cerr << "Unknown LOG_LEVEL environment variable value: " << level_str
                      << ". Defaulting to INFO." << std::endl;
            return LogLevel::INFO;
        }

        std::string getCurrentTimestamp ()
        {
            auto now = std::time(nullptr);
            std::tm tm_buf;
        #ifdef _WIN32
            localtime_s(&tm_buf, &now);
        #else // POSIX
            localtime_r(&now, &tm_buf);
        #endif
            std::ostringstream oss;
            oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        namespace { // Anonymous namespace for internal linkage
            struct LoggerState {
                std::ofstream log_file_stream;
                std::mutex log_mutex;
                LogLevel min_log_level;

                LoggerState() : min_log_level(getLogLevelFromEnv()) {
                    log_file_stream.open("netquack.log", std::ios_base::app);
                    if (!log_file_stream.is_open()) {
                        std::cerr << "Failed to open log file: netquack.log" << std::endl;
                    }
                }

                // Optional: Destructor to close file if needed
                // ~LoggerState() {
                //     if (log_file_stream.is_open()) {
                //         log_file_stream.close();
                //     }
                // }
            };

            LoggerState& getLoggerState() {
                static LoggerState instance; // Meyers singleton
                return instance;
            }
        } // anonymous namespace

        void LogMessage (LogLevel level, const std::string &message)
        {
            LoggerState &logger = getLoggerState();

            if (level < logger.min_log_level) {
                return;
            }

            const char *level_str = "";
            switch (level) {
                case LogLevel::DEBUG: level_str = "DEBUG"; break;
                case LogLevel::INFO: level_str = "INFO"; break;
                case LogLevel::WARNING: level_str = "WARNING"; break;
                case LogLevel::ERROR: level_str = "ERROR"; break;
                case LogLevel::CRITICAL: level_str = "CRITICAL"; break;
            }

            std::string timestamp = getCurrentTimestamp(); // Get timestamp once

            // Lock for file operations and critical section
            std::lock_guard<std::mutex> guard(logger.log_mutex);

            if (logger.log_file_stream.is_open()) {
                 logger.log_file_stream << "[" << timestamp << "] "
                                   << level_str << " - "
                                   << message << std::endl;
            }

            if (level >= LogLevel::ERROR) {
                // std::cerr is generally thread-safe at the line level, but ensuring timestamp consistency
                std::cerr << "[" << timestamp << "] [" << level_str << "] " << message << std::endl;
            }

            if (level == LogLevel::CRITICAL) {
                if (logger.log_file_stream.is_open()) {
                    logger.log_file_stream.flush(); // Ensure critical log is written
                }
                throw std::runtime_error(message);
            }
        }
    } // namespace netquack
} // namespace duckdb
