// Copyright 2025 Arash Hatami

#include "utils.hpp"

#include <curl/curl.h>

#ifdef _WIN32
#include <windows.h>
#else // POSIX
#include <sys/stat.h>
#endif

#include "logger.hpp"

namespace duckdb {
namespace netquack {
bool file_exists(const char *file_path) {
#ifdef _WIN32
	DWORD attributes = GetFileAttributesA(file_path);
	return (attributes != INVALID_FILE_ATTRIBUTES);
#else // POSIX
	struct stat buffer;
	return (stat(file_path, &buffer) == 0);
#endif
}

CURL *CreateCurlHandler(curl_write_callback write_callback) {
	CURL *curl = curl_easy_init();
	if (!curl) {
		LogMessage(LogLevel::LOG_CRITICAL, "Failed to initialize CURL");
	}

	const char *ca_info = std::getenv("CURL_CA_INFO");
#if !defined(_WIN32) && !defined(__APPLE__)
	if (!ca_info) {
		// Check for common CA certificate bundle locations on Linux
		for (const auto *path : {
		         "/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
		         "/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
		         "/etc/ssl/ca-bundle.pem",                            // OpenSUSE
		         "/etc/pki/tls/cacert.pem",                           // OpenELEC
		         "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
		         "/etc/ssl/cert.pem"                                  // Alpine Linux
		     }) {
			if (file_exists(path)) {
				ca_info = path;
				break;
			}
		}
	}
#endif
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

	if (ca_info) {
		// Set the custom CA certificate bundle file
		// https://github.com/hatamiarash7/duckdb-netquack/issues/6
		LogMessage(LogLevel::LOG_DEBUG, "Using custom CA certificate bundle: " + std::string(ca_info));
		curl_easy_setopt(curl, CURLOPT_CAINFO, ca_info);
	}
	const char *ca_path = std::getenv("CURL_CA_PATH");
	if (ca_path) {
		// Set the custom CA certificate directory
		LogMessage(LogLevel::LOG_DEBUG, "Using custom CA certificate directory: " + std::string(ca_path));
		curl_easy_setopt(curl, CURLOPT_CAPATH, ca_path);
	}

	return curl;
}

size_t WriteStringCallback(char *contents, size_t size, size_t nmemb, void *userp) {
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

size_t WriteFileCallback(char *contents, size_t size, size_t nmemb, void *userp) {
	FILE *file = (FILE *)userp;
	return fwrite(contents, size, nmemb, file);
}
} // namespace netquack
} // namespace duckdb
