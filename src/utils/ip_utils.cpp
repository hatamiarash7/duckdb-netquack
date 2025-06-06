// Copyright 2025 Arash Hatami

#include "ip_utils.hpp"

#include <regex>

namespace duckdb
{
    namespace netquack
    {
        IPInfo IPCalculator::calculate (const std::string &ipWithMask)
        {
            // Validate input format
            if (!isValidInput (ipWithMask))
            {
                throw std::invalid_argument ("Invalid input format. Expected format: x.x.x.x[/x]");
            }

            // Parse IP and subnet mask
            size_t slashPos = ipWithMask.find ('/');
            std::string ip  = ipWithMask.substr (0, slashPos);

            // Default to /24 if no subnet mask is provided
            int maskBits = 24;
            if (slashPos != std::string::npos)
            {
                std::string maskStr = ipWithMask.substr (slashPos + 1);
                try
                {
                    maskBits = std::stoi (maskStr);
                }
                catch (const std::exception &)
                {
                    throw std::invalid_argument ("Invalid subnet mask. Must be a number between 0 and 32.");
                }
            }

            // Validate subnet mask
            if (maskBits < 0 || maskBits > 32)
            {
                throw std::invalid_argument ("Subnet mask must be between 0 and 32");
            }

            // Validate IP address
            if (!isValidIP (ip))
            {
                throw std::invalid_argument ("Invalid IP address.");
            }

            // Calculate network properties
            std::string subnetMask   = getSubnetMask (maskBits);
            std::string wildcardMask = getWildcardMask (subnetMask);

            IPInfo info;
            info.address     = ip;
            info.netmask     = subnetMask;
            info.wildcard    = wildcardMask;
            info.hostsPerNet = getHostsPerNet (maskBits);
            info.ipClass     = getIPClass (ip);

            // Store the network IP without CIDR suffix temporarily for calculations
            std::string network_ip_only = getNetworkAddress(ip, subnetMask, maskBits);
            info.network = network_ip_only + "/" + std::to_string(maskBits); // Store with CIDR

            if (maskBits == 32) {
                info.broadcast = "-"; // No traditional broadcast
                info.hostMin   = network_ip_only;  // The IP itself is the only host
                info.hostMax   = network_ip_only;  // The IP itself is the only host
            } else if (maskBits == 31) {
                // For /31, the two addresses in the range are the network and broadcast addresses,
                // and both are considered usable hosts (RFC 3021).
                info.hostMin   = network_ip_only; // First IP in /31
                info.broadcast = getBroadcastAddress(network_ip_only, wildcardMask); // Calculate actual broadcast (second IP)
                info.hostMax   = info.broadcast; // The second IP in the /31 range
            } else { // For masks /0 to /30
                info.broadcast = getBroadcastAddress(network_ip_only, wildcardMask);
                info.hostMin   = getHostMin(network_ip_only);
                info.hostMax   = getHostMax(info.broadcast);
            }

            return info;
        }

        bool IPCalculator::isValidInput (const std::string &input)
        {
            static const std::regex pattern (R"((\d{1,3}\.){3}\d{1,3}(/\d{1,2})?)");
            return std::regex_match (input, pattern);
        }

        bool IPCalculator::isValidIP (const std::string &ip)
        {
            auto octets = parseIP (ip);
            for (int octet : octets)
            {
                if (octet < 0 || octet > 255)
                {
                    return false;
                }
            }
            return true;
        }

        std::array<int, 4> IPCalculator::parseIP (const std::string &ip)
        {
            std::array<int, 4> octets{};
            std::stringstream ss (ip);
            std::string octet;
            for (int i = 0; i < 4; ++i)
            {
                std::getline (ss, octet, '.');
                octets[i] = std::stoi (octet);
            }
            return octets;
        }

        std::string IPCalculator::getSubnetMask (int maskBits)
        {
            uint32_t mask = 0xFFFFFFFF << (32 - maskBits);
            return intToIP (mask);
        }

        std::string IPCalculator::getWildcardMask (const std::string &subnetMask)
        {
            auto mask = parseIP (subnetMask);
            std::string wildcard;
            for (int i = 0; i < 4; ++i)
            {
                wildcard += std::to_string (255 - mask[i]) + (i < 3 ? "." : "");
            }
            return wildcard;
        }

        // Note: maskBits parameter is changed from const int& to int
        std::string IPCalculator::getNetworkAddress (const std::string &ip, const std::string &subnetMask, int maskBits)
        {
            auto ipOctets   = parseIP (ip);
            auto maskOctets = parseIP (subnetMask);
            std::string network_ip_str;
            for (int i = 0; i < 4; ++i)
            {
                network_ip_str += std::to_string (ipOctets[i] & maskOctets[i]) + (i < 3 ? "." : "");
            }
            // Return only the IP string, not with /maskBits
            return network_ip_str;
        }

        std::string IPCalculator::getBroadcastAddress (const std::string &networkAddress, const std::string &wildcardMask)
        {
            auto networkOctets  = parseIP (networkAddress);
            auto wildcardOctets = parseIP (wildcardMask);
            std::string broadcast;
            for (int i = 0; i < 4; ++i)
            {
                broadcast += std::to_string (networkOctets[i] | wildcardOctets[i]) + (i < 3 ? "." : "");
            }
            return broadcast;
        }

        std::string IPCalculator::getHostMin (const std::string &networkAddress)
        {
            auto octets = parseIP (networkAddress);
            octets[3] += 1;
            return intToIP (octets);
        }

        std::string IPCalculator::getHostMax (const std::string &broadcastAddress)
        {
            auto octets = parseIP (broadcastAddress);
            octets[3] -= 1;
            return intToIP (octets);
        }

        int IPCalculator::getHostsPerNet (int maskBits)
        {
            // Basic validation, though typically done before calling this
            if (maskBits < 0 || maskBits > 32) {
                throw std::invalid_argument("Subnet mask must be between 0 and 32");
            }

            if (maskBits == 32) {
                return 1; // The IP itself is considered the only host
            }
            if (maskBits == 31) {
                return 2; // For point-to-point links, as per RFC 3021
            }
            // For networks /0 through /30
            // Number of addresses in the subnet is 2^(32-maskBits)
            // Subtract 2 for network and broadcast addresses
            uint64_t total_ips = 1ULL << (32 - maskBits);
            return static_cast<int>(total_ips - 2);
        }

        std::string IPCalculator::getIPClass (const std::string &ip)
        {
            auto octets    = parseIP (ip);
            int firstOctet = octets[0];

            if (firstOctet >= 1 && firstOctet <= 126)
                return "A";
            if (firstOctet == 127)
                return "A, Loopback";
            if (firstOctet >= 128 && firstOctet <= 191)
                return "B";
            if (firstOctet >= 192 && firstOctet <= 223)
                return "C";
            if (firstOctet >= 224 && firstOctet <= 239)
                return "D";
            return "E";
        }

        std::string IPCalculator::intToIP (uint32_t ip)
        {
            std::stringstream ss;
            ss << ((ip >> 24) & 0xFF) << "." << ((ip >> 16) & 0xFF) << "." << ((ip >> 8) & 0xFF) << "." << (ip & 0xFF);
            return ss.str ();
        }

        std::string IPCalculator::intToIP (const std::array<int, 4> &octets)
        {
            std::stringstream ss;
            for (int i = 0; i < 4; ++i)
            {
                ss << octets[i] << (i < 3 ? "." : "");
            }
            return ss.str ();
        }
    } // namespace netquack
} // namespace duckdb
