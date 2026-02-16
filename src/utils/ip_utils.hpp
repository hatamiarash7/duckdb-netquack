// Copyright 2025 Arash Hatami

#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "duckdb.hpp"

namespace duckdb {
namespace netquack {
struct IPInfo {
	std::string address;
	std::string netmask;
	std::string wildcard;
	std::string network;
	std::string hostMin;
	std::string hostMax;
	std::string broadcast;
	int64_t hostsPerNet;
	std::string ipClass;
};

class IPCalculator {
public:
	static IPInfo calculate(const std::string &ipWithMask);

private:
	static bool isValidInput(const std::string &input);
	static bool isValidIP(const std::string &ip);
	static std::array<int, 4> parseIP(const std::string &ip);
	static std::string getSubnetMask(int maskBits);
	static std::string getWildcardMask(const std::string &subnetMask);
	static std::string getNetworkAddress(const std::string &ip, const std::string &subnetMask, int maskBits);
	static std::string getBroadcastAddress(const std::string &networkAddress, const std::string &wildcardMask);
	static std::string getHostMin(const std::string &networkAddress);
	static std::string getHostMax(const std::string &broadcastAddress);
	static std::string getHostMinForP2P(const std::string &networkAddress);
	static std::string getHostMaxForP2P(const std::string &networkAddress);
	static int64_t getHostsPerNet(int maskBits);
	static std::string getIPClass(const std::string &ip);
	static std::string intToIP(uint32_t ip);
	static std::string intToIP(const std::array<int, 4> &octets);
};
} // namespace netquack
} // namespace duckdb
