// Copyright 2025 Arash Hatami

#include "utils.hpp"

#include <curl/curl.h>

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else // POSIX
#include <sys/stat.h>
#endif

#include "logger.hpp"

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

        CURL *CreateCurlHandler (curl_write_callback write_callback)
        {
            CURL *curl = curl_easy_init ();
            if (!curl)
            {
                LogMessage (LogLevel::LOG_CRITICAL, "Failed to initialize CURL");
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
            curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_callback);
            
            if (ca_info)
            {
                // Set the custom CA certificate bundle file
                // https://github.com/hatamiarash7/duckdb-netquack/issues/6
                LogMessage (LogLevel::LOG_DEBUG, "Using custom CA certificate bundle: " + std::string (ca_info));
                curl_easy_setopt (curl, CURLOPT_CAINFO, ca_info);
            }
            const char *ca_path = std::getenv ("CURL_CA_PATH");
            if (ca_path)
            {
                // Set the custom CA certificate directory
                LogMessage (LogLevel::LOG_DEBUG, "Using custom CA certificate directory: " + std::string (ca_path));
                curl_easy_setopt (curl, CURLOPT_CAPATH, ca_path);
            }

            return curl;
        }

        size_t WriteStringCallback (char *contents, size_t size, size_t nmemb, void *userp)
        {
            ((std::string *)userp)->append ((char *)contents, size * nmemb);
            return size * nmemb;
        }

        size_t WriteFileCallback (char *contents, size_t size, size_t nmemb, void *userp)
        {
            FILE *file = (FILE *)userp;
            return fwrite (contents, size, nmemb, file);
        }

        std::string DownloadPublicSuffixList ()
        {
            CURL *curl = CreateCurlHandler (WriteStringCallback);
            CURLcode res;
            std::string readBuffer;

            curl_easy_setopt (curl, CURLOPT_URL, "https://publicsuffix.org/list/public_suffix_list.dat");
            curl_easy_setopt (curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform (curl);
            curl_easy_cleanup (curl);

            if (res != CURLE_OK)
            {
                LogMessage (LogLevel::LOG_ERROR, std::string (curl_easy_strerror (res)));
                LogMessage (LogLevel::LOG_CRITICAL, "Failed to download public suffix list. Check logs for details.");
            }

            return readBuffer;
        }

        // Function to parse the public suffix list and store it in a table
        void LoadPublicSuffixList (DatabaseInstance &db, bool force)
        {
            // Check if the table already exists
            Connection con (db);
            auto table_exists = con.Query ("SELECT 1 FROM information_schema.tables WHERE table_name = 'public_suffix_list'");
            auto table_data   = con.Query ("SELECT * FROM public_suffix_list");

            if (table_exists->RowCount () == 0 || table_data->RowCount () <= 1 || force)
            {
                LogMessage (LogLevel::LOG_INFO, "Loading public suffix list...");
                // Download the list
                auto list_data = DownloadPublicSuffixList ();

                // Validate the downloaded data
                if (list_data.empty ())
                {
                    LogMessage (LogLevel::LOG_CRITICAL, "Failed to download public suffix list: empty data received");
                }

                // Count non-comment/non-empty lines for validation
                std::istringstream validation_stream (list_data);
                std::string validation_line;
                size_t valid_line_count = 0;

                while (std::getline (validation_stream, validation_line))
                {
                    if (!validation_line.empty () && validation_line[0] != '/' && validation_line[0] != ' ')
                    {
                        valid_line_count++;
                    }
                }

                if (valid_line_count <= 1)
                {
                    LogMessage (LogLevel::LOG_ERROR, validation_line);
                    LogMessage (LogLevel::LOG_CRITICAL, "Downloaded public suffix list contains no valid entries. Try again or run `SELECT update_suffixes();`.");
                }

                LogMessage (LogLevel::LOG_INFO, "Downloaded public suffix list with " + std::to_string (valid_line_count) + " valid entries");

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
