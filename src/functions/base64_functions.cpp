// Copyright 2026 Arash Hatami

#include "base64_functions.hpp"

#include <array>

namespace duckdb {

// Base64 alphabet
static const char BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Reverse lookup table: maps ASCII byte value to Base64 index (255 = invalid)
static constexpr std::array<uint8_t, 256> BuildBase64DecodeTable() {
	std::array<uint8_t, 256> table = {};
	for (auto &v : table) {
		v = 255;
	}
	for (uint8_t i = 0; i < 64; i++) {
		const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		table[static_cast<uint8_t>(chars[i])] = i;
	}
	return table;
}

static constexpr auto BASE64_DECODE_TABLE = BuildBase64DecodeTable();

void Base64EncodeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();

		try {
			auto encoded = netquack::Base64Encode(input);
			result_data[i] = StringVector::AddString(result, encoded);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error encoding base64: " + std::string(e.what()));
		}
	}
}

void Base64DecodeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();

		try {
			auto decoded = netquack::Base64Decode(input);
			result_data[i] = StringVector::AddString(result, decoded);
		} catch (const std::exception &e) {
			result_data[i] = StringVector::AddString(result, "Error decoding base64: " + std::string(e.what()));
		}
	}
}

namespace netquack {

std::string Base64Encode(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	const auto *data = reinterpret_cast<const uint8_t *>(input.data());
	size_t len = input.size();

	// Calculate output size: 4 output chars per 3 input bytes, rounded up
	size_t output_len = 4 * ((len + 2) / 3);
	std::string result;
	result.reserve(output_len);

	for (size_t i = 0; i < len; i += 3) {
		uint32_t octet_a = data[i];
		uint32_t octet_b = (i + 1 < len) ? data[i + 1] : 0;
		uint32_t octet_c = (i + 2 < len) ? data[i + 2] : 0;

		uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

		result += BASE64_CHARS[(triple >> 18) & 0x3F];
		result += BASE64_CHARS[(triple >> 12) & 0x3F];
		result += (i + 1 < len) ? BASE64_CHARS[(triple >> 6) & 0x3F] : '=';
		result += (i + 2 < len) ? BASE64_CHARS[triple & 0x3F] : '=';
	}

	return result;
}

std::string Base64Decode(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	// Strip whitespace (spaces, tabs, newlines, carriage returns)
	std::string cleaned;
	cleaned.reserve(input.size());
	for (char c : input) {
		if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
			cleaned += c;
		}
	}

	if (cleaned.empty()) {
		return "";
	}

	// Validate length (must be multiple of 4 for standard base64)
	if (cleaned.size() % 4 != 0) {
		return "INVALID_BASE64";
	}

	// Validate characters
	for (size_t i = 0; i < cleaned.size(); i++) {
		char c = cleaned[i];
		if (c == '=') {
			// Padding only allowed at the end (last 1-2 chars)
			if (i < cleaned.size() - 2) {
				return "INVALID_BASE64";
			}
		} else if (BASE64_DECODE_TABLE[static_cast<uint8_t>(c)] == 255) {
			return "INVALID_BASE64";
		}
	}

	// Count padding
	size_t padding = 0;
	if (!cleaned.empty() && cleaned[cleaned.size() - 1] == '=') {
		padding++;
	}
	if (cleaned.size() > 1 && cleaned[cleaned.size() - 2] == '=') {
		padding++;
	}

	// Calculate output size
	size_t output_len = (cleaned.size() / 4) * 3 - padding;
	std::string result;
	result.reserve(output_len);

	for (size_t i = 0; i < cleaned.size(); i += 4) {
		uint32_t sextet_a = BASE64_DECODE_TABLE[static_cast<uint8_t>(cleaned[i])];
		uint32_t sextet_b = BASE64_DECODE_TABLE[static_cast<uint8_t>(cleaned[i + 1])];
		uint32_t sextet_c = (cleaned[i + 2] == '=') ? 0 : BASE64_DECODE_TABLE[static_cast<uint8_t>(cleaned[i + 2])];
		uint32_t sextet_d = (cleaned[i + 3] == '=') ? 0 : BASE64_DECODE_TABLE[static_cast<uint8_t>(cleaned[i + 3])];

		uint32_t triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

		result += static_cast<char>((triple >> 16) & 0xFF);
		if (cleaned[i + 2] != '=') {
			result += static_cast<char>((triple >> 8) & 0xFF);
		}
		if (cleaned[i + 3] != '=') {
			result += static_cast<char>(triple & 0xFF);
		}
	}

	return result;
}

} // namespace netquack
} // namespace duckdb
