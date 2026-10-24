// Copyright 2026 Arash Hatami

#include "public_suffix_functions.hpp"

#include <algorithm>
#include <string>

#include "../utils/tld_lookup.hpp"

namespace duckdb {

template <bool (*Predicate)(const std::string_view &)>
static void BooleanDomainFunction(DataChunk &args, Vector &result) {
	const auto &input_vector = args.data[0];
	auto result_data = netquack_compat::MutableData<bool>(result);
	auto &result_validity = netquack_compat::MutableValidity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		result_data[i] = Predicate(input);
	}
}

void IsPublicSuffixFunction(DataChunk &args, ExpressionState &, Vector &result) {
	BooleanDomainFunction<netquack::IsPublicSuffix>(args, result);
}

void IsKnownTLDFunction(DataChunk &args, ExpressionState &, Vector &result) {
	BooleanDomainFunction<netquack::IsKnownTLD>(args, result);
}

namespace netquack {
// Lowercase and strip a single leading and trailing dot (".com", "co.uk.")
static std::string NormalizeSuffix(const std::string_view &input) {
	std::string_view view = input;
	if (!view.empty() && view.front() == '.') {
		view.remove_prefix(1);
	}
	if (!view.empty() && view.back() == '.') {
		view.remove_suffix(1);
	}

	std::string normalized(view);
	std::transform(normalized.begin(), normalized.end(), normalized.begin(),
	               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return normalized;
}

bool IsPublicSuffix(const std::string_view &input) {
	return isPublicSuffix(NormalizeSuffix(input));
}

bool IsKnownTLD(const std::string_view &input) {
	return isKnownTLD(NormalizeSuffix(input));
}
} // namespace netquack
} // namespace duckdb
