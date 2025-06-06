// Copyright 2025 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb
{
    namespace netquack
    {
        struct IPInfo
        {
            std::string address;
            std::string netmask;
            std::string wildcard;
            std::string network;
            std::string hostMin;
            std::string hostMax;
            std::string broadcast;
            int hostsPerNet;
            std::string ipClass;
        };

        class IPCalculator
        {
          public:
            static IPInfo calculate (const std::string &ipWithMask);

          private:
            static bool isValidInput (const std::string &input);
            static bool isValidIP (const std::string &ip); // Already effectively const
            static std::array<int, 4> parseIP (const std::string &ip); // ip is not modified
            static std::string getSubnetMask (int maskBits);
            static std::string getWildcardMask (const std::string &subnetMask);
            static std::string getNetworkAddress (const std::string &ip, const std::string &subnetMask, int maskBits); // pass int by value
            static std::string getBroadcastAddress (const std::string &networkAddress, const std::string &wildcardMask);
            static std::string getHostMin (const std::string &networkAddress);
            static std::string getHostMax (const std::string &broadcastAddress);
            static int getHostsPerNet (int maskBits);
            static std::string getIPClass (const std::string &ip);
            static std::string intToIP (uint32_t ip); // Already by value
            static std::string intToIP (const std::array<int, 4> &octets); // octets is not modified
        };
    } // namespace netquack
} // namespace duckdb
