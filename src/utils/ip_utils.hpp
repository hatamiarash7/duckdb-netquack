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
            static bool isValidIP (const std::string &ip);
            static std::vector<int> parseIP (const std::string &ip);
            static std::string getSubnetMask (int maskBits);
            static std::string getWildcardMask (const std::string &subnetMask);
            static std::string getNetworkAddress (const std::string &ip, const std::string &subnetMask, const int &maskBits);
            static std::string getBroadcastAddress (const std::string &networkAddress, const std::string &wildcardMask);
            static std::string getHostMin (const std::string &networkAddress);
            static std::string getHostMax (const std::string &broadcastAddress);
            static int getHostsPerNet (int maskBits);
            static std::string getIPClass (const std::string &ip);
            static std::string intToIP (uint32_t ip);
            static std::string intToIP (const std::vector<int> &octets);
        };
    } // namespace netquack
} // namespace duckdb
