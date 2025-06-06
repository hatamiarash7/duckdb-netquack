// Copyright 2025 Arash Hatami

#include "get_tranco.hpp"

#include <curl/curl.h>

#include <fstream>
#include <regex>

#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

namespace duckdb
{
    namespace netquack
    {
        // Function to get the download code for the Tranco list
        std::string GetTrancoDownloadCode (char *date)
        {
            CURL *curl = CreateCurlHandler ();
            CURLcode res;
            std::string readBuffer;

            // Construct the URL for the daily list
            std::string url = "https://tranco-list.eu/daily_list?date=" + std::string (date) + "&subdomains=true";

            LogMessage (LogLevel::INFO, "Get Tranco download code for date: " + std::string (date));

            curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
            curl_easy_setopt (curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform (curl);
            curl_easy_cleanup (curl);

            if (res != CURLE_OK)
            {
                LogMessage (LogLevel::ERROR, std::string (curl_easy_strerror (res)));
                LogMessage (LogLevel::CRITICAL, "Failed to fetch Tranco download code.");
            }

            // Extract the download code from the URL
            std::regex code_regex (R"(Information on the Tranco list with ID ([A-Z0-9]+))");
            std::smatch code_match;
            if (std::regex_search (readBuffer, code_match, code_regex) && code_match.size () > 1)
            {
                LogMessage (LogLevel::INFO, "Tranco download code: " + code_match[1].str ());
                return code_match[1].str ();
            }

            LogMessage (LogLevel::CRITICAL, "Failed to extract Tranco download code.");
        }

        // Function to download the Tranco list and create a table
        void LoadTrancoList (DatabaseInstance &db, bool force)
        {
            // Get yesterday's date in YYYY-MM-DD format
            std::time_t now    = std::time (nullptr);
            std::tm *yesterday = std::localtime (&now);
            yesterday->tm_mday -= 1; // Subtract one day
            std::mktime (yesterday); // Normalize the time
            char date[11];
            std::strftime (date, sizeof (date), "%Y-%m-%d", yesterday);

            // Construct the file name
            std::string temp_file = "tranco_list_" + std::string (date) + ".csv";

            // Need file_exists from utils.hpp - ensure it's available.
            // #include "../utils/utils.hpp" should already be there.
            // #include "../utils/logger.hpp" for LogMessage

            bool file_needs_download = false;
            if (force) {
                LogMessage(LogLevel::INFO, "Force update for Tranco list: " + temp_file);
                if (file_exists(temp_file.c_str())) {
                    if (remove(temp_file.c_str()) != 0) {
                        LogMessage(LogLevel::WARNING, "Failed to remove existing Tranco file during force update: " + temp_file + ". Proceeding with download attempt.");
                    }
                }
                file_needs_download = true;
            } else {
                if (!file_exists(temp_file.c_str())) {
                    LogMessage(LogLevel::INFO, "Tranco list " + temp_file + " not found locally.");
                    file_needs_download = true;
                } else {
                    LogMessage(LogLevel::INFO, "Found existing Tranco list: " + temp_file + ". Use force=true to re-download.");
                }
            }

            if (file_needs_download) {
                LogMessage(LogLevel::INFO, "Attempting to download Tranco list to: " + temp_file);
                std::string download_code = GetTrancoDownloadCode(date);

                if (download_code.empty()) {
                    LogMessage(LogLevel::CRITICAL, "Failed to obtain Tranco download code for date: " + std::string(date) + ". Aborting download.");
                    return; // Critical log would throw, but explicit return for clarity.
                }

                std::string download_url = "https://tranco-list.eu/download/" + download_code + "/full";
                LogMessage(LogLevel::INFO, "Downloading Tranco list from: " + download_url);

                CURL *curl = CreateCurlHandler(); // Assuming CreateCurlHandler is available
                CURLcode res;
                FILE *fp = fopen(temp_file.c_str(), "wb"); // Changed variable name from 'file' to 'fp' to avoid clash
                if (!fp) {
                    if(curl) curl_easy_cleanup(curl); // Cleanup curl if fp failed
                    LogMessage(LogLevel::CRITICAL, "Failed to create temporary file for Tranco list: " + temp_file);
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, download_url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                res = curl_easy_perform(curl);

                fclose(fp); // Close file pointer
                curl_easy_cleanup(curl);

                if (res != CURLE_OK) {
                    remove(temp_file.c_str()); // Clean up the temporary file on download error
                    LogMessage(LogLevel::ERROR, "CURL error downloading Tranco list: " + std::string(curl_easy_strerror(res)));
                    LogMessage(LogLevel::CRITICAL, "Failed to download Tranco list. Check logs for details.");
                    return;
                }
                LogMessage(LogLevel::INFO, "Tranco list downloaded successfully: " + temp_file);
            }

            // Final check before trying to use the file
            if (!file_exists(temp_file.c_str())) {
                LogMessage(LogLevel::CRITICAL, "Tranco list `" + temp_file + "` not found or download failed. If you forced update, check logs. Otherwise, try `SELECT update_tranco(true);`");
                return;
            }

            // Parse the CSV data and insert into a table
            LogMessage (LogLevel::INFO, "Inserting Tranco list into table");

            Connection con (db);
            string query = "CREATE OR REPLACE TABLE tranco_list AS"
                           " SELECT rank,"
                           " domain,"
                           " CASE"
                           " WHEN rank <= 1000 THEN 'top1k'"
                           " WHEN rank <= 5000 THEN 'top5k'"
                           " WHEN rank <= 10000 THEN 'top10k'"
                           " WHEN rank <= 50000 THEN 'top50k'"
                           " WHEN rank <= 100000 THEN 'top100k'"
                           " WHEN rank <= 500000 THEN 'top500k'"
                           " WHEN rank <= 1000000 THEN 'top1m'"
                           " WHEN rank <= 5000000 THEN 'top5m'"
                           " ELSE 'other'"
                           " END AS category"
                           " FROM read_csv('" +
                temp_file + "', header = false, columns = { 'rank': 'INTEGER', 'domain': 'VARCHAR' })";
            auto result = con.Query (query);

            if (result->HasError ())
            {
                LogMessage (LogLevel::CRITICAL, result->GetError ());
            }
        }

        // Function to update the Tranco list table
        void UpdateTrancoListFunction (DataChunk &args, ExpressionState &state, Vector &result)
        {
            // Extract the force_download argument
            auto &force_download_vector = args.data[0];
            bool force_download         = force_download_vector.GetValue (0).GetValue<bool> ();

            // Load the Tranco list into the database
            auto &db = *state.GetContext ().db;
            netquack::LoadTrancoList (db, force_download);

            result.SetValue (0, Value ("Tranco list updated"));
        }

        // Function to get the Tranco rank of a domain
        void GetTrancoRankFunction (DataChunk &args, ExpressionState &state, Vector &result)
        {
            auto &db = *state.GetContext ().db;
            netquack::LoadPublicSuffixList (db, false);
            Connection con (db);

            auto table_exists = con.Query ("SELECT 1 FROM information_schema.tables WHERE table_name = 'tranco_list'");

            if (table_exists->RowCount () == 0)
            {
                LogMessage (LogLevel::CRITICAL, "Tranco table not found. Download it first using `SELECT update_tranco(true);`");
                // Add this loop to set all results to NULL and return:
                for (idx_t i = 0; i < args.size(); i++) {
                    FlatVector::SetNull(result, i, true);
                }
                return; // Exit the function early
            }

            // Extract the input from the arguments
            auto &input_vector = args.data[0];
            auto result_data   = FlatVector::GetData<string_t> (result);

            for (idx_t i = 0; i < args.size (); i++)
            {
                auto input = input_vector.GetValue (i).ToString ();
                std::transform (input.begin (), input.end (), input.begin (), ::tolower);

                try
                {
                    auto query = "SELECT rank FROM tranco_list WHERE domain = '" + input + "'";

                    auto query_result = con.Query (query);
                    auto rank         = query_result->RowCount () > 0 ? query_result->GetValue (0, 0) : Value ();

                    if (rank.IsNull()) {
                        FlatVector::SetNull(result, i, true);
                    } else {
                        result_data[i] = StringVector::AddString (result, rank.ToString ());
                    }
                }
                catch (const std::exception &e)
                {
                    // Set NULL on error
                    FlatVector::SetNull (result, i, true);
                }
            }
        }

        // Function to get the Tranco rank category of a domain
        void GetTrancoRankCategoryFunction (DataChunk &args, ExpressionState &state, Vector &result)
        {
            auto &db = *state.GetContext ().db;
            netquack::LoadPublicSuffixList (db, false);
            Connection con (db);

            auto table_exists = con.Query ("SELECT 1 FROM information_schema.tables WHERE table_name = 'tranco_list'");

            if (table_exists->RowCount () == 0)
            {
                LogMessage (LogLevel::CRITICAL, "Tranco table not found. Download it first using `SELECT update_tranco(true);`");
                // Add this loop to set all results to NULL and return:
                for (idx_t i = 0; i < args.size(); i++) {
                    FlatVector::SetNull(result, i, true);
                }
                return; // Exit the function early
            }

            // Extract the input from the arguments
            auto &input_vector = args.data[0];
            auto result_data   = FlatVector::GetData<string_t> (result);

            for (idx_t i = 0; i < args.size (); i++)
            {
                auto input = input_vector.GetValue (i).ToString ();
                std::transform (input.begin (), input.end (), input.begin (), ::tolower);

                try
                {
                    auto query = "SELECT category FROM tranco_list WHERE domain = '" + input + "'";

                    auto query_result = con.Query (query);
                    auto category     = query_result->RowCount () > 0 ? query_result->GetValue (0, 0) : Value ();

                    if (category.IsNull()) {
                        FlatVector::SetNull(result, i, true);
                    } else {
                        result_data[i] = StringVector::AddString (result, category.ToString ());
                    }
                }
                catch (const std::exception &e)
                {
                    // Set NULL on error
                    FlatVector::SetNull (result, i, true);
                }
            }
        }
    } // namespace netquack
} // namespace duckdb
