#pragma once

#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

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
            static std::string getNetworkAddress (const std::string &ip, const std::string &subnetMask);
            static std::string getBroadcastAddress (const std::string &networkAddress, const std::string &wildcardMask);
            static std::string getHostMin (const std::string &networkAddress);
            static std::string getHostMax (const std::string &broadcastAddress);
            static int getHostsPerNet (int maskBits);
            static std::string getIPClass (const std::string &ip);
            static std::string intToIP (uint32_t ip);
            static std::string intToIP (const std::vector<int> &octets);
        };

        struct IPCalcFunc
        {
            static unique_ptr<FunctionData> Bind (ClientContext &context, TableFunctionBindInput &input, vector<LogicalType> &return_types, vector<string> &names);
            static void Scan (ClientContext &context, TableFunctionInput &data_p, DataChunk &output);
            static unique_ptr<LocalTableFunctionState> InitLocal (ExecutionContext &context, TableFunctionInitInput &input, GlobalTableFunctionState *global_state_p);
            static unique_ptr<GlobalTableFunctionState> InitGlobal (ClientContext &context, TableFunctionInitInput &input);
        };
    } // namespace netquack
} // namespace duckdb
