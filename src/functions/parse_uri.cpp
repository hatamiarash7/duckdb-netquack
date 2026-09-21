// Copyright 2026 Arash Hatami

#include "parse_uri.hpp"

#include "../utils/url_helpers.hpp"

namespace duckdb {

void ParseURIFunction(DataChunk &args, ExpressionState &, Vector &result) {
	const auto &input_vector = args.data[0];
	auto &child_entries = StructVector::GetEntries(result);
	auto &result_validity = FlatVector::Validity(result);

	auto scheme_data = FlatVector::GetData<string_t>(*child_entries[0]);
	auto host_data = FlatVector::GetData<string_t>(*child_entries[1]);
	auto port_data = FlatVector::GetData<string_t>(*child_entries[2]);
	auto path_data = FlatVector::GetData<string_t>(*child_entries[3]);
	auto query_data = FlatVector::GetData<string_t>(*child_entries[4]);
	auto fragment_data = FlatVector::GetData<string_t>(*child_entries[5]);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			for (auto &child : child_entries) {
				FlatVector::SetNull(*child, i, true);
			}
			continue;
		}

		auto input = value.ToString();
		try {
			auto parsed = netquack::ParseURI(input);
			scheme_data[i] = StringVector::AddString(*child_entries[0], parsed.scheme);
			host_data[i] = StringVector::AddString(*child_entries[1], parsed.host);
			port_data[i] = StringVector::AddString(*child_entries[2], parsed.port);
			path_data[i] = StringVector::AddString(*child_entries[3], parsed.path);
			query_data[i] = StringVector::AddString(*child_entries[4], parsed.query);
			fragment_data[i] = StringVector::AddString(*child_entries[5], parsed.fragment);
		} catch (const std::exception &) {
			scheme_data[i] = StringVector::AddString(*child_entries[0], "");
			host_data[i] = StringVector::AddString(*child_entries[1], "");
			port_data[i] = StringVector::AddString(*child_entries[2], "");
			path_data[i] = StringVector::AddString(*child_entries[3], "/");
			query_data[i] = StringVector::AddString(*child_entries[4], "");
			fragment_data[i] = StringVector::AddString(*child_entries[5], "");
		}
	}
}

namespace netquack {

static void AsciiToLower(std::string &s) {
	for (char &c : s) {
		if (c >= 'A' && c <= 'Z') {
			c = static_cast<char>(c - 'A' + 'a');
		}
	}
}

static void ParsePort(const char *pos, const char *end, ParsedURI &result) {
	if (pos >= end) {
		return;
	}
	for (const char *p = pos; p < end; ++p) {
		if (!isNumericASCII(*p)) {
			return;
		}
	}
	result.port.assign(pos, end - pos);
}

static void ParseHostPort(const char *pos, const char *end, ParsedURI &result) {
	if (pos >= end) {
		return;
	}

	if (*pos == '[') {
		const char *closing = pos + 1;
		while (closing < end && *closing != ']') {
			++closing;
		}
		if (closing < end) {
			result.host.assign(pos, (closing + 1) - pos);
			AsciiToLower(result.host);
			pos = closing + 1;
			if (pos < end && *pos == ':') {
				ParsePort(pos + 1, end, result);
			}
			return;
		}
		result.host.assign(pos, end - pos);
		AsciiToLower(result.host);
		return;
	}

	const char *colon = nullptr;
	for (const char *p = pos; p < end; ++p) {
		if (*p == ':') {
			colon = p;
			break;
		}
	}

	if (colon) {
		result.host.assign(pos, colon - pos);
		AsciiToLower(result.host);
		ParsePort(colon + 1, end, result);
	} else {
		result.host.assign(pos, end - pos);
		AsciiToLower(result.host);
	}
}

static void ParseAuthority(const char *pos, const char *end, ParsedURI &result) {
	const char *at = nullptr;
	for (const char *p = pos; p < end; ++p) {
		if (*p == '/') {
			break;
		}
		if (*p == '@') {
			at = p;
		}
	}
	if (at) {
		pos = at + 1;
	}

	const char *path_start = end;
	for (const char *p = pos; p < end; ++p) {
		if (*p == '/') {
			path_start = p;
			break;
		}
	}

	ParseHostPort(pos, path_start, result);

	if (path_start < end) {
		result.path.assign(path_start, end - path_start);
	} else {
		result.path = "/";
	}
}

static bool LooksLikeAuthority(const char *pos, const char *end) {
	if (pos >= end) {
		return false;
	}
	if (*pos == '[') {
		return true;
	}

	const char *limit = pos;
	while (limit < end && *limit != '/') {
		++limit;
	}
	if (limit == pos) {
		return false;
	}

	for (const char *p = pos; p < limit; ++p) {
		if (*p == '.' || *p == ':') {
			return true;
		}
	}
	return false;
}

static bool EqualsIgnoreCase(const char *begin, const char *end, const char *literal) {
	for (; begin < end && *literal; ++begin, ++literal) {
		char c = *begin;
		if (c >= 'A' && c <= 'Z') {
			c = static_cast<char>(c - 'A' + 'a');
		}
		if (c != *literal) {
			return false;
		}
	}
	return begin == end && *literal == '\0';
}

static bool IsOpaqueScheme(const char *begin, const char *end) {
	return EqualsIgnoreCase(begin, end, "mailto") || EqualsIgnoreCase(begin, end, "tel") ||
	       EqualsIgnoreCase(begin, end, "sms") || EqualsIgnoreCase(begin, end, "data") ||
	       EqualsIgnoreCase(begin, end, "javascript") || EqualsIgnoreCase(begin, end, "about") ||
	       EqualsIgnoreCase(begin, end, "blob") || EqualsIgnoreCase(begin, end, "magnet") ||
	       EqualsIgnoreCase(begin, end, "sip") || EqualsIgnoreCase(begin, end, "sips") ||
	       EqualsIgnoreCase(begin, end, "urn") || EqualsIgnoreCase(begin, end, "news");
}

static bool LooksLikePort(const char *pos, const char *end) {
	if (pos >= end || !isNumericASCII(*pos)) {
		return false;
	}
	for (const char *p = pos; p < end; ++p) {
		if (*p == '/' || *p == '?' || *p == '#') {
			return true;
		}
		if (!isNumericASCII(*p)) {
			return false;
		}
	}
	return true;
}

static const char *FindSchemeColon(const char *pos, const char *end) {
	if (pos >= end || !isAlphaASCII(*pos)) {
		return nullptr;
	}
	const char *p = pos + 1;
	bool has_dot = false;
	while (p < end && (isAlphaNumericASCII(*p) || *p == '+' || *p == '-' || *p == '.')) {
		if (*p == '.') {
			has_dot = true;
		}
		++p;
	}
	if (p >= end || *p != ':') {
		return nullptr;
	}
	const char *after = p + 1;
	if (end - after >= 2 && after[0] == '/' && after[1] == '/') {
		return p;
	}
	if (has_dot) {
		return nullptr;
	}
	if (LooksLikePort(after, end) && !IsOpaqueScheme(pos, p)) {
		return nullptr;
	}
	return p;
}

ParsedURI ParseURI(const std::string_view &input) {
	ParsedURI result;
	if (input.empty()) {
		return result;
	}

	const char *pos = input.data();
	const char *end = pos + input.size();

	const char *hash = find_first_symbols<'#'>(pos, end);
	if (hash != end) {
		result.fragment.assign(hash + 1, end - (hash + 1));
		end = hash;
	}

	const char *qmark = find_first_symbols<'?'>(pos, end);
	if (qmark != end) {
		result.query.assign(qmark + 1, end - (qmark + 1));
		end = qmark;
	}

	if (end - pos >= 2 && pos[0] == '/' && pos[1] == '/') {
		ParseAuthority(pos + 2, end, result);
		return result;
	}

	const char *scheme_colon = FindSchemeColon(pos, end);
	if (scheme_colon) {
		result.scheme.assign(pos, scheme_colon - pos);
		AsciiToLower(result.scheme);
		pos = scheme_colon + 1;
		if (end - pos >= 2 && pos[0] == '/' && pos[1] == '/') {
			ParseAuthority(pos + 2, end, result);
		} else if (pos < end) {
			result.path.assign(pos, end - pos);
		} else {
			result.path.clear();
		}
		return result;
	}

	if (LooksLikeAuthority(pos, end)) {
		ParseAuthority(pos, end, result);
	} else if (pos < end) {
		result.path.assign(pos, end - pos);
	}
	return result;
}

} // namespace netquack
} // namespace duckdb
