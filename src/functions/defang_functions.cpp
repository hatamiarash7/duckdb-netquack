// Copyright 2026 Arash Hatami

#include "defang_functions.hpp"

#include "ip_functions.hpp"

namespace duckdb {

void DefangFunction(DataChunk &args, ExpressionState &, Vector &result) {
	const auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		auto defanged = netquack::Defang(input);
		result_data[i] = StringVector::AddString(result, defanged);
	}
}

void RefangFunction(DataChunk &args, ExpressionState &, Vector &result) {
	const auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		auto refanged = netquack::Refang(input);
		result_data[i] = StringVector::AddString(result, refanged);
	}
}

namespace netquack {

static char ToLowerAscii(char c) {
	return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

static bool IsAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Case-insensitive match of `word` (lowercase) at `pos`
static bool MatchAt(const std::string_view &input, size_t pos, const std::string_view &word) {
	if (pos + word.size() > input.size()) {
		return false;
	}
	for (size_t k = 0; k < word.size(); k++) {
		if (ToLowerAscii(input[pos + k]) != word[k]) {
			return false;
		}
	}
	return true;
}

static bool IsWordStart(const std::string_view &input, size_t pos) {
	return pos == 0 || !IsAlpha(input[pos - 1]);
}

// Length of a scheme `prefix` (+ optional 's') at `pos` when followed by "://", otherwise 0
static size_t MatchScheme(const std::string_view &input, size_t pos, const std::string_view &prefix,
                          bool allow_secure) {
	if (!IsWordStart(input, pos) || !MatchAt(input, pos, prefix)) {
		return 0;
	}
	size_t end = pos + prefix.size();
	if (allow_secure && end < input.size() && ToLowerAscii(input[end]) == 's') {
		end++;
	}
	return MatchAt(input, end, "://") ? end - pos : 0;
}

// Replaces the characters at `offsets` within the scheme, preserving letter case
static void AppendScheme(std::string &out, const std::string_view &scheme, char replacement,
                         std::initializer_list<size_t> offsets) {
	std::string replaced(scheme);
	for (auto off : offsets) {
		bool upper = replaced[off] >= 'A' && replaced[off] <= 'Z';
		replaced[off] = upper ? static_cast<char>(replacement - 'a' + 'A') : replacement;
	}
	out += replaced;
}

static bool IsIPv6Char(char c) {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == ':' || c == '.';
}

static bool IsAlnum(char c) {
	return IsAlpha(c) || (c >= '0' && c <= '9');
}

// Length of a valid IPv6 address starting at `pos` (not preceded/followed by address characters), otherwise 0
static size_t MatchIPv6(const std::string_view &input, size_t pos) {
	if (pos > 0 && (IsIPv6Char(input[pos - 1]) || IsAlpha(input[pos - 1]))) {
		return 0;
	}
	size_t end = pos;
	size_t colons = 0;
	while (end < input.size() && IsIPv6Char(input[end])) {
		colons += input[end] == ':';
		end++;
	}
	if (colons < 2 || (end < input.size() && IsAlnum(input[end]))) {
		return 0;
	}
	return IsValidIPv6(std::string(input.substr(pos, end - pos))) ? end - pos : 0;
}

static bool IsBracketed(const std::string_view &input, size_t pos) {
	return pos > 0 && input[pos - 1] == '[' && pos + 1 < input.size() && input[pos + 1] == ']';
}

std::string Defang(const std::string_view &input) {
	std::string result;
	result.reserve(input.size() * 2);

	for (size_t i = 0; i < input.size(); i++) {
		size_t len;
		if ((len = MatchScheme(input, i, "http", true)) > 0) {
			AppendScheme(result, input.substr(i, len), 'x', {1, 2});
			i += len - 1;
		} else if ((len = MatchScheme(input, i, "ftp", false)) > 0) {
			AppendScheme(result, input.substr(i, len), 'x', {1});
			i += len - 1;
		} else if ((len = MatchIPv6(input, i)) > 0) {
			for (char c : input.substr(i, len)) {
				if (c == ':') {
					result += "[:]";
				} else if (c == '.') {
					result += "[.]";
				} else {
					result += c;
				}
			}
			i += len - 1;
		} else if (input[i] == '.' || input[i] == '@') {
			if (IsBracketed(input, i)) {
				result += input[i];
			} else {
				result += '[';
				result += input[i];
				result += ']';
			}
		} else {
			result += input[i];
		}
	}

	return result;
}

static char ClosingBracket(char open) {
	switch (open) {
	case '[':
		return ']';
	case '(':
		return ')';
	case '{':
		return '}';
	default:
		return '\0';
	}
}

std::string Refang(const std::string_view &input) {
	struct Token {
		std::string_view inner;
		std::string_view replacement;
	};
	static constexpr Token TOKENS[] = {{"://", "://"}, {".", "."}, {"dot", "."}, {":", ":"},
	                                   {"@", "@"},     {"at", "@"}};

	// Pass 1: unwrap bracketed separators like [.], (dot), {:}, [://], [@]
	std::string unwrapped;
	unwrapped.reserve(input.size());
	for (size_t i = 0; i < input.size(); i++) {
		char close = ClosingBracket(input[i]);
		bool matched = false;
		if (close != '\0') {
			for (const auto &token : TOKENS) {
				size_t close_pos = i + 1 + token.inner.size();
				if (close_pos < input.size() && input[close_pos] == close && MatchAt(input, i + 1, token.inner)) {
					unwrapped += token.replacement;
					i = close_pos;
					matched = true;
					break;
				}
			}
		}
		if (!matched) {
			unwrapped += input[i];
		}
	}

	// Pass 2: restore defanged schemes (hxxp, hxxps, fxp)
	std::string_view view(unwrapped);
	std::string result;
	result.reserve(unwrapped.size());
	for (size_t i = 0; i < view.size(); i++) {
		size_t len;
		if ((len = MatchScheme(view, i, "hxxp", true)) > 0) {
			AppendScheme(result, view.substr(i, len), 't', {1, 2});
			i += len - 1;
		} else if ((len = MatchScheme(view, i, "fxp", false)) > 0) {
			AppendScheme(result, view.substr(i, len), 't', {1});
			i += len - 1;
		} else {
			result += view[i];
		}
	}

	return result;
}

} // namespace netquack
} // namespace duckdb
