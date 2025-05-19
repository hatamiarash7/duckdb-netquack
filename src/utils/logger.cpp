#include "logger.hpp"

#include <fstream>
#include <iostream>

namespace duckdb
{
    namespace netquack
    {
        void LogMessage (const std::string &level, const std::string &message)
        {
            std::ofstream log_file ("netquack.log", std::ios_base::app);
            log_file << "[" << level << "] " << message << std::endl;

            if (level == "ERROR")
            {
                throw std::runtime_error (message);
            }
        }
    } // namespace netquack
} // namespace duckdb
