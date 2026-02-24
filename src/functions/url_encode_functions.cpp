// Copyright 2026 Arash Hatami

#include "url_encode_functions.hpp"

#include <array>
#include <sstream>

namespace duckdb {

// RFC 3986 unreserved characters: A-Z a-z 0-9 - _ . ~
// These are the ONLY characters that are NOT percent-encoded.
static constexpr std::array<bool, 256> BuildUnreservedTable() {
	std::array<bool, 256> table = {};
	for (auto &v : table) {
		v = false;
	}
	// A-Z
	for (int c = 'A'; c <= 'Z'; c++) {
		table[c] = true;
	}
	// a-z
	for (int c = 'a'; c <= 'z'; c++) {
		table[c] = true;
	}
	// 0-9
	for (int c = '0'; c <= '9'; c++) {
		table[c] = true;
	}
	// - _ . ~
	table[static_cast<uint8_t>('-')] = true;
	table[static_cast<uint8_t>('_')] = true;
	table[static_cast<uint8_t>('.')] = true;
	table[static_cast<uint8_t>('~')] = true;
	return table;
}

static constexpr auto UNRESERVED_TABLE = BuildUnreservedTable();

static const char HEX_DIGITS[] = "0123456789ABCDEF";

// Returns -1 for invalid hex character
static int HexVal(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return -1;
}

void UrlEncodeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
		auto encoded = netquack::UrlEncode(input);
		result_data[i] = StringVector::AddString(result, encoded);
	}
}

void UrlDecodeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
		auto decoded = netquack::UrlDecode(input);
		result_data[i] = StringVector::AddString(result, decoded);
	}
}

namespace netquack {

std::string UrlEncode(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	std::string result;
	// Worst case: every byte becomes %XX (3x expansion)
	result.reserve(input.size() * 3);

	for (unsigned char c : input) {
		if (UNRESERVED_TABLE[c]) {
			result += static_cast<char>(c);
		} else {
			result += '%';
			result += HEX_DIGITS[(c >> 4) & 0x0F];
			result += HEX_DIGITS[c & 0x0F];
		}
	}

	return result;
}

std::string UrlDecode(const std::string &input) {
	if (input.empty()) {
		return "";
	}

	std::string result;
	result.reserve(input.size());

	for (size_t i = 0; i < input.size(); i++) {
		if (input[i] == '%' && i + 2 < input.size()) {
			int hi = HexVal(input[i + 1]);
			int lo = HexVal(input[i + 2]);
			if (hi >= 0 && lo >= 0) {
				result += static_cast<char>((hi << 4) | lo);
				i += 2;
				continue;
			}
			// Invalid hex digits — pass through the '%' literally
			result += '%';
		} else if (input[i] == '+') {
			// '+' is commonly used as space in application/x-www-form-urlencoded
			result += ' ';
		} else {
			result += input[i];
		}
	}

	return result;
}

} // namespace netquack
} // namespace duckdb
