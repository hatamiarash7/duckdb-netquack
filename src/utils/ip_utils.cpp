// Copyright 2025 Arash Hatami

#include "ip_utils.hpp"

#include <array>
#include <charconv>
#include <stdexcept>
#include <sstream>

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
                int parsed_mask = 0;
                auto result = std::from_chars (maskStr.data (), maskStr.data () + maskStr.size (), parsed_mask);
                if (result.ec != std::errc {} || result.ptr != maskStr.data () + maskStr.size ())
                {
                    throw std::invalid_argument ("Invalid subnet mask. Must be a number between 0 and 32.");
                }
                maskBits = parsed_mask;
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

            if (maskBits == 32)
            {
                // For /32, the IP itself is the only host (Hostroute)
                info.network   = ip;
                info.broadcast = "-";
                info.hostMin   = "-";
                info.hostMax   = "-";
            }
            else if (maskBits == 31)
            {
                // For /31, RFC 3021 point-to-point links (no broadcast)
                info.network   = getNetworkAddress (ip, subnetMask, maskBits);
                info.broadcast = "-";
                info.hostMin   = getHostMinForP2P (info.network);
                info.hostMax   = getHostMaxForP2P (info.network);
            }
            else
            {
                info.network   = getNetworkAddress (ip, subnetMask, maskBits);
                info.broadcast = getBroadcastAddress (info.network, wildcardMask);
                info.hostMin   = getHostMin (info.network);
                info.hostMax   = getHostMax (info.broadcast);
            }

            return info;
        }

        bool IPCalculator::isValidInput (const std::string &input)
        {
            if (input.empty () || input.length () > 18) // max: xxx.xxx.xxx.xxx/xx
                return false;

            size_t slashPos = input.find ('/');
            std::string ip_part = (slashPos != std::string::npos) ? input.substr (0, slashPos) : input;

            // Check basic structure
            int dotCount = 0;
            for (char c : ip_part)
            {
                if (c == '.')
                    dotCount++;
                else if (c < '0' || c > '9')
                    return false;
            }
            if (dotCount != 3)
                return false;

            // Check mask part if present
            if (slashPos != std::string::npos)
            {
                std::string mask_part = input.substr (slashPos + 1);
                if (mask_part.empty () || mask_part.length () > 2)
                    return false;
                for (char c : mask_part)
                {
                    if (c < '0' || c > '9')
                        return false;
                }
            }

            return true;
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
            // Handle edge case for /0 (shifting by 32 is undefined behavior)
            if (maskBits == 0)
            {
                return "0.0.0.0";
            }
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

        std::string IPCalculator::getNetworkAddress (const std::string &ip, const std::string &subnetMask, int maskBits)
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

        std::string IPCalculator::getHostMinForP2P (const std::string &networkAddress)
        {
            // For /31 networks (RFC 3021), return the network address itself
            auto octets = parseIP (networkAddress);
            return intToIP (octets);
        }

        std::string IPCalculator::getHostMaxForP2P (const std::string &networkAddress)
        {
            // For /31 networks (RFC 3021), return network + 1
            auto octets = parseIP (networkAddress);
            octets[3] += 1;
            return intToIP (octets);
        }

        int64_t IPCalculator::getHostsPerNet (int maskBits)
        {
            if (maskBits == 32)
                return 1; // Special case for /32
            if (maskBits == 31)
                return 2; // RFC 3021 point-to-point links
            // Use 64-bit arithmetic to avoid overflow for small masks like /0, /1
            return (1LL << (32 - maskBits)) - 2;
        }

        std::string IPCalculator::getIPClass (const std::string &ip)
        {
            auto octets    = parseIP (ip);
            int firstOctet = octets[0];

            if (firstOctet == 0)
                return "E"; // 0.x.x.x is reserved
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
