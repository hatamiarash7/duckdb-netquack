#include "utils.hpp"

#include <curl/curl.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else // POSIX
#include <sys/stat.h>
#endif

namespace duckdb
{
    namespace netquack
    {
        bool file_exists (const char *file_path)
        {
#ifdef _WIN32
            DWORD attributes = GetFileAttributesA (file_path);
            return (attributes != INVALID_FILE_ATTRIBUTES);
#else // POSIX
            struct stat buffer;
            return (stat (file_path, &buffer) == 0);
#endif
        }

        CURL *CreateCurlHandler ()
        {
            CURL *curl = curl_easy_init ();
            if (!curl)
            {
                throw std::runtime_error ("Failed to initialize CURL");
            }

            const char *ca_info = std::getenv ("CURL_CA_INFO");
#if !defined(_WIN32) && !defined(__APPLE__)
            if (!ca_info)
            {
                // Check for common CA certificate bundle locations on Linux
                for (const auto *path : {
                         "/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
                         "/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
                         "/etc/ssl/ca-bundle.pem",                            // OpenSUSE
                         "/etc/pki/tls/cacert.pem",                           // OpenELEC
                         "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
                         "/etc/ssl/cert.pem"                                  // Alpine Linux
                     })
                {
                    if (file_exists (path))
                    {
                        ca_info = path;
                        break;
                    }
                }
            }
#endif
            curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
            curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            if (ca_info)
            {
                // Set the custom CA certificate bundle file
                // https://github.com/hatamiarash7/duckdb-netquack/issues/6
                LogMessage ("DEBUG", "Using custom CA certificate bundle: " + std::string (ca_info));
                curl_easy_setopt (curl, CURLOPT_CAINFO, ca_info);
            }
            const char *ca_path = std::getenv ("CURL_CA_PATH");
            if (ca_path)
            {
                // Set the custom CA certificate directory
                LogMessage ("DEBUG", "Using custom CA certificate directory: " + std::string (ca_path));
                curl_easy_setopt (curl, CURLOPT_CAPATH, ca_path);
            }

            return curl;
        }

        void LogMessage (const std::string &level, const std::string &message)
        {
            std::ofstream log_file ("netquack.log", std::ios_base::app);
            log_file << "[" << level << "] " << message << std::endl;

            if (level == "ERROR")
            {
                throw std::runtime_error (message);
            }
        }

        size_t WriteCallback (void *contents, size_t size, size_t nmemb, void *userp)
        {
            ((std::string *)userp)->append ((char *)contents, size * nmemb);
            return size * nmemb;
        }

        std::string DownloadPublicSuffixList ()
        {
            CURL *curl = CreateCurlHandler ();
            CURLcode res;
            std::string readBuffer;

            curl_easy_setopt (curl, CURLOPT_URL, "https://publicsuffix.org/list/public_suffix_list.dat");
            curl_easy_setopt (curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform (curl);
            curl_easy_cleanup (curl);

            if (res != CURLE_OK)
            {
                LogMessage ("ERROR", "Failed to download public suffix list: " + std::string (curl_easy_strerror (res)));
                throw std::runtime_error ("Failed to download public suffix list. Check logs for details.");
            }

            return readBuffer;
        }

        // Function to parse the public suffix list and store it in a table
        void LoadPublicSuffixList (DatabaseInstance &db, bool force)
        {
            // Check if the table already exists
            Connection con (db);
            auto table_exists = con.Query ("SELECT 1 FROM information_schema.tables WHERE table_name = 'public_suffix_list'");

            if (table_exists->RowCount () == 0 || force)
            {
                // Download the list
                auto list_data = DownloadPublicSuffixList ();

                // Parse the list and insert into a table
                std::istringstream stream (list_data);
                std::string line;
                con.Query ("CREATE OR REPLACE TABLE public_suffix_list (suffix VARCHAR)");

                while (std::getline (stream, line))
                {
                    // Skip comments and empty lines
                    if (line.empty () || line[0] == '/' || line[0] == ' ')
                        continue;

                    // Replace `*.` with an empty string
                    size_t wildcard_pos = line.find ("*.");
                    if (wildcard_pos != std::string::npos)
                    {
                        line.replace (wildcard_pos, 2, "");
                    }

                    // Insert the suffix into the table
                    con.Query ("INSERT INTO public_suffix_list (suffix) VALUES ('" + line + "')");
                }
            }
        }

        // Function to update the public suffix list table
        void UpdateSuffixesFunction (DataChunk &args, ExpressionState &state, Vector &result)
        {
            // Load the public suffix list if not already loaded
            auto &db = *state.GetContext ().db;

            // Load the public suffix list with the update flag set to true
            LoadPublicSuffixList (db, true);

            result.SetValue (0, Value ("updated"));
        }
    } // namespace netquack
} // namespace duckdb
