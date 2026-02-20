// Copyright 2025 Arash Hatami

#include "ip_functions.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace duckdb {
namespace netquack {

// ---------------------------------------------------------------------------
// IPv4 validation
// ---------------------------------------------------------------------------
bool IsValidIPv4(const std::string &ip) {
	if (ip.empty() || ip.size() > 15) {
		return false;
	}

	int dot_count = 0;
	int octet_start = 0;

	for (size_t i = 0; i <= ip.size(); ++i) {
		if (i == ip.size() || ip[i] == '.') {
			int len = static_cast<int>(i) - octet_start;
			if (len == 0 || len > 3) {
				return false;
			}

			// Check all digits
			for (int j = octet_start; j < static_cast<int>(i); ++j) {
				if (ip[j] < '0' || ip[j] > '9') {
					return false;
				}
			}

			// No leading zeros (except for "0" itself)
			if (len > 1 && ip[octet_start] == '0') {
				return false;
			}

			// Parse and range-check
			int val = 0;
			auto result = std::from_chars(ip.data() + octet_start, ip.data() + i, val);
			if (result.ec != std::errc {} || val < 0 || val > 255) {
				return false;
			}

			if (ip[i] == '.') {
				dot_count++;
				if (dot_count > 3) {
					return false;
				}
			}

			octet_start = static_cast<int>(i) + 1;
		}
	}

	return dot_count == 3;
}

// ---------------------------------------------------------------------------
// IPv6 validation
// ---------------------------------------------------------------------------
static bool IsValidIPv6Group(const std::string &group) {
	if (group.empty() || group.size() > 4) {
		return false;
	}
	for (char c : group) {
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
			return false;
		}
	}
	return true;
}

bool IsValidIPv6(const std::string &ip) {
	if (ip.empty()) {
		return false;
	}

	// Strip surrounding brackets if present: [::1] -> ::1
	std::string addr = ip;
	if (addr.front() == '[' && addr.back() == ']') {
		addr = addr.substr(1, addr.size() - 2);
	}

	if (addr.empty()) {
		return false;
	}

	// Handle IPv4-mapped IPv6 (e.g. ::ffff:192.168.1.1)
	size_t last_colon = addr.rfind(':');
	if (last_colon != std::string::npos) {
		std::string after_last = addr.substr(last_colon + 1);
		if (after_last.find('.') != std::string::npos) {
			// The part after the last colon looks like an IPv4 address
			if (!IsValidIPv4(after_last)) {
				return false;
			}
			// Replace the IPv4 portion with two valid hex groups for counting
			addr = addr.substr(0, last_colon + 1) + "0:0";
		}
	}

	// Split by ':'
	std::vector<std::string> parts;
	std::string part;
	std::istringstream stream(addr);
	while (std::getline(stream, part, ':')) {
		parts.push_back(part);
	}

	// Handle leading :: (addr starts with ::)
	if (addr.size() >= 2 && addr[0] == ':' && addr[1] == ':') {
		parts.insert(parts.begin(), "");
	}
	// Handle trailing :: (addr ends with ::)
	if (addr.size() >= 2 && addr[addr.size() - 1] == ':' && addr[addr.size() - 2] == ':') {
		parts.emplace_back("");
	}

	// Count double-colon occurrences
	int double_colon_count = 0;
	{
		size_t pos = 0;
		while ((pos = addr.find("::", pos)) != std::string::npos) {
			double_colon_count++;
			pos += 2;
		}
	}

	if (double_colon_count > 1) {
		return false;
	}

	if (double_colon_count == 1) {
		// Find where the :: is
		int empty_count = 0;
		for (const auto &p : parts) {
			if (p.empty()) {
				empty_count++;
			}
		}

		// Validate non-empty parts
		for (const auto &p : parts) {
			if (!p.empty() && !IsValidIPv6Group(p)) {
				return false;
			}
		}

		// With ::, we need at most 8 groups total
		int non_empty = static_cast<int>(parts.size()) - empty_count;
		if (non_empty > 7) {
			return false;
		}

		return true;
	}

	// No double-colon: must have exactly 8 groups
	if (parts.size() != 8) {
		return false;
	}
	for (const auto &p : parts) {
		if (!IsValidIPv6Group(p)) {
			return false;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// IP version detection
// ---------------------------------------------------------------------------
int DetectIPVersion(const std::string &ip) {
	if (ip.empty()) {
		return 0;
	}

	// If it contains ':', it's potentially IPv6
	if (ip.find(':') != std::string::npos) {
		return IsValidIPv6(ip) ? 6 : 0;
	}

	// Otherwise try IPv4
	return IsValidIPv4(ip) ? 4 : 0;
}

// ---------------------------------------------------------------------------
// IPv4 <-> integer conversion
// ---------------------------------------------------------------------------
uint32_t IPv4ToUint32(const std::string &ip) {
	uint32_t result = 0;
	int shift = 24;
	int octet_start = 0;

	for (size_t i = 0; i <= ip.size(); ++i) {
		if (i == ip.size() || ip[i] == '.') {
			int val = 0;
			std::from_chars(ip.data() + octet_start, ip.data() + i, val);
			result |= (static_cast<uint32_t>(val) << shift);
			shift -= 8;
			octet_start = static_cast<int>(i) + 1;
		}
	}

	return result;
}

std::string Uint32ToIPv4(uint32_t ip) {
	return std::to_string((ip >> 24) & 0xFF) + "." + std::to_string((ip >> 16) & 0xFF) + "." +
	       std::to_string((ip >> 8) & 0xFF) + "." + std::to_string(ip & 0xFF);
}

// ---------------------------------------------------------------------------
// Private/reserved IPv4 range check
// ---------------------------------------------------------------------------
bool IsPrivateIPv4(const std::string &ip) {
	uint32_t addr = IPv4ToUint32(ip);

	// 10.0.0.0/8 (10.0.0.0 - 10.255.255.255) - RFC 1918
	if ((addr & 0xFF000000) == 0x0A000000) {
		return true;
	}

	// 172.16.0.0/12 (172.16.0.0 - 172.31.255.255) - RFC 1918
	if ((addr & 0xFFF00000) == 0xAC100000) {
		return true;
	}

	// 192.168.0.0/16 (192.168.0.0 - 192.168.255.255) - RFC 1918
	if ((addr & 0xFFFF0000) == 0xC0A80000) {
		return true;
	}

	// 127.0.0.0/8 (127.0.0.0 - 127.255.255.255) - Loopback
	if ((addr & 0xFF000000) == 0x7F000000) {
		return true;
	}

	// 169.254.0.0/16 (169.254.0.0 - 169.254.255.255) - Link-local (APIPA)
	if ((addr & 0xFFFF0000) == 0xA9FE0000) {
		return true;
	}

	// 0.0.0.0/8 (0.0.0.0 - 0.255.255.255) - Current network
	if ((addr & 0xFF000000) == 0x00000000) {
		return true;
	}

	// 100.64.0.0/10 (100.64.0.0 - 100.127.255.255) - Carrier-grade NAT (RFC 6598)
	if ((addr & 0xFFC00000) == 0x64400000) {
		return true;
	}

	// 192.0.0.0/24 (192.0.0.0 - 192.0.0.255) - IETF Protocol Assignments
	if ((addr & 0xFFFFFF00) == 0xC0000000) {
		return true;
	}

	// 192.0.2.0/24 (192.0.2.0 - 192.0.2.255) - Documentation (TEST-NET-1)
	if ((addr & 0xFFFFFF00) == 0xC0000200) {
		return true;
	}

	// 198.51.100.0/24 (198.51.100.0 - 198.51.100.255) - Documentation (TEST-NET-2)
	if ((addr & 0xFFFFFF00) == 0xC6336400) {
		return true;
	}

	// 203.0.113.0/24 (203.0.113.0 - 203.0.113.255) - Documentation (TEST-NET-3)
	if ((addr & 0xFFFFFF00) == 0xCB007100) {
		return true;
	}

	// 198.18.0.0/15 (198.18.0.0 - 198.19.255.255) - Benchmarking (RFC 2544)
	if ((addr & 0xFFFE0000) == 0xC6120000) {
		return true;
	}

	// 224.0.0.0/4 (224.0.0.0 - 239.255.255.255) - Multicast
	if ((addr & 0xF0000000) == 0xE0000000) {
		return true;
	}

	// 240.0.0.0/4 (240.0.0.0 - 255.255.255.254) - Reserved for future use
	if ((addr & 0xF0000000) == 0xF0000000) {
		return true;
	}

	// 255.255.255.255/32 - Broadcast
	if (addr == 0xFFFFFFFF) {
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// Private/reserved IPv6 range check
// ---------------------------------------------------------------------------
static std::array<uint16_t, 8> ParseIPv6Groups(const std::string &ip) {
	std::array<uint16_t, 8> groups {};
	std::string addr = ip;

	// Strip brackets
	if (addr.front() == '[' && addr.back() == ']') {
		addr = addr.substr(1, addr.size() - 2);
	}

	// Handle :: expansion
	size_t dc_pos = addr.find("::");
	std::vector<std::string> left_parts, right_parts;

	if (dc_pos != std::string::npos) {
		std::string left = addr.substr(0, dc_pos);
		std::string right = addr.substr(dc_pos + 2);

		if (!left.empty()) {
			std::istringstream ls(left);
			std::string p;
			while (std::getline(ls, p, ':')) {
				left_parts.push_back(p);
			}
		}
		if (!right.empty()) {
			std::istringstream rs(right);
			std::string p;
			while (std::getline(rs, p, ':')) {
				right_parts.push_back(p);
			}
		}

		size_t left_count = left_parts.size();
		size_t right_count = right_parts.size();
		size_t zeros_needed = 8 - left_count - right_count;

		size_t idx = 0;
		for (size_t i = 0; i < left_count; ++i) {
			groups[idx++] = static_cast<uint16_t>(std::stoul(left_parts[i], nullptr, 16));
		}
		for (size_t i = 0; i < zeros_needed; ++i) {
			groups[idx++] = 0;
		}
		for (size_t i = 0; i < right_count; ++i) {
			groups[idx++] = static_cast<uint16_t>(std::stoul(right_parts[i], nullptr, 16));
		}
	} else {
		std::istringstream stream(addr);
		std::string part;
		int idx = 0;
		while (std::getline(stream, part, ':') && idx < 8) {
			groups[idx++] = static_cast<uint16_t>(std::stoul(part, nullptr, 16));
		}
	}

	return groups;
}

bool IsPrivateIPv6(const std::string &ip) {
	auto groups = ParseIPv6Groups(ip);

	// ::1 - Loopback
	if (groups[0] == 0 && groups[1] == 0 && groups[2] == 0 && groups[3] == 0 && groups[4] == 0 && groups[5] == 0 &&
	    groups[6] == 0 && groups[7] == 1) {
		return true;
	}

	// :: - Unspecified address
	bool all_zero = true;
	for (int i = 0; i < 8; i++) {
		if (groups[i] != 0) {
			all_zero = false;
			break;
		}
	}
	if (all_zero) {
		return true;
	}

	// fe80::/10 - Link-local
	if ((groups[0] & 0xFFC0) == 0xFE80) {
		return true;
	}

	// fc00::/7 - Unique local address (ULA)
	if ((groups[0] & 0xFE00) == 0xFC00) {
		return true;
	}

	// ff00::/8 - Multicast
	if ((groups[0] & 0xFF00) == 0xFF00) {
		return true;
	}

	// ::ffff:0:0/96 - IPv4-mapped IPv6 addresses (check the mapped IPv4 part)
	if (groups[0] == 0 && groups[1] == 0 && groups[2] == 0 && groups[3] == 0 && groups[4] == 0 && groups[5] == 0xFFFF) {
		// Reconstruct the IPv4 address and check if it's private
		uint32_t ipv4 = (static_cast<uint32_t>(groups[6]) << 16) | static_cast<uint32_t>(groups[7]);
		std::string ipv4_str = Uint32ToIPv4(ipv4);
		return IsPrivateIPv4(ipv4_str);
	}

	// 2001:db8::/32 - Documentation
	if (groups[0] == 0x2001 && groups[1] == 0x0DB8) {
		return true;
	}

	// 100::/64 - Discard prefix (RFC 6666)
	if (groups[0] == 0x0100 && groups[1] == 0 && groups[2] == 0 && groups[3] == 0) {
		return true;
	}

	return false;
}

} // namespace netquack

// ===========================================================================
// DuckDB Scalar Function implementations
// ===========================================================================

void IsValidIPFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<bool>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		result_data[i] = netquack::IsValidIPv4(input) || netquack::IsValidIPv6(input);
	}
}

void IsPrivateIPFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<bool>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		int version = netquack::DetectIPVersion(input);
		if (version == 4) {
			result_data[i] = netquack::IsPrivateIPv4(input);
		} else if (version == 6) {
			result_data[i] = netquack::IsPrivateIPv6(input);
		} else {
			// Invalid IP: return NULL
			result_validity.SetInvalid(i);
		}
	}
}

void IPToIntFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<uint64_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		if (!netquack::IsValidIPv4(input)) {
			// Only support IPv4 for integer conversion (IPv6 needs HUGEINT)
			result_validity.SetInvalid(i);
			continue;
		}

		result_data[i] = static_cast<uint64_t>(netquack::IPv4ToUint32(input));
	}
}

void IntToIPFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		uint64_t int_val = value.GetValue<uint64_t>();
		if (int_val > 0xFFFFFFFF) {
			// Out of IPv4 range
			result_validity.SetInvalid(i);
			continue;
		}

		auto ip_str = netquack::Uint32ToIPv4(static_cast<uint32_t>(int_val));
		result_data[i] = StringVector::AddString(result, ip_str);
	}
}

void IPVersionFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &input_vector = args.data[0];
	auto result_data = FlatVector::GetData<int8_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	for (idx_t i = 0; i < args.size(); i++) {
		auto value = input_vector.GetValue(i);
		if (value.IsNull()) {
			result_validity.SetInvalid(i);
			continue;
		}

		auto input = value.ToString();
		int version = netquack::DetectIPVersion(input);
		if (version == 0) {
			// Invalid IP: return NULL
			result_validity.SetInvalid(i);
		} else {
			result_data[i] = static_cast<int8_t>(version);
		}
	}
}

} // namespace duckdb
