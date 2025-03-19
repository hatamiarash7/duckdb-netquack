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

            if (maskBits != 32)
            {
                info.network   = getNetworkAddress (ip, subnetMask, maskBits);
                info.broadcast = getBroadcastAddress (info.network, wildcardMask);
                info.hostMin   = getHostMin (info.network);
                info.hostMax   = getHostMax (info.broadcast);
            }
            else
            {
                // For /32, the IP itself is the only host (Hostroute)
                info.network = ip;
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

        std::string IPCalculator::getNetworkAddress (const std::string &ip, const std::string &subnetMask, const int &maskBits)
        {
            auto ipOctets   = parseIP (ip);
            auto maskOctets = parseIP (subnetMask);
            std::string network;
            for (int i = 0; i < 4; ++i)
            {
                network += std::to_string (ipOctets[i] & maskOctets[i]) + (i < 3 ? "." : "");
            }
            return network + "/" + std::to_string (maskBits);
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
            if (maskBits == 32)
                return 1; // Special case for /32
            return (1 << (32 - maskBits)) - 2;
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
