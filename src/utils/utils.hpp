// Copyright 2025 Arash Hatami

#pragma once

#include <curl/curl.h>

#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/execution/expression_executor.hpp"
#include "duckdb/main/database.hpp"

namespace duckdb
{
    namespace netquack
    {
        // Function to get a CURL handler with custom write callback
        CURL *CreateCurlHandler (curl_write_callback write_callback);

        // Function to write data to a string (for HTTP responses)
        size_t WriteStringCallback (char *contents, size_t size, size_t nmemb, void *userp);

        // Function to write data to a file (for file downloads)
        size_t WriteFileCallback (char *contents, size_t size, size_t nmemb, void *userp);


        // Function to download the Tranco list and create a table
        void LoadTrancoList (DatabaseInstance &db, bool force);
    } // namespace netquack
} // namespace duckdb
