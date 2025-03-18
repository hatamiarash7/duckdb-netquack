#include "ipcalc.hpp"

#include <cmath>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "../utils/utils.hpp"

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

            // Default to /32 if no subnet mask is provided
            int maskBits = 32;
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
                info.network   = getNetworkAddress (ip, subnetMask);
                info.broadcast = getBroadcastAddress (info.network, wildcardMask);
                info.hostMin   = getHostMin (info.network);
                info.hostMax   = getHostMax (info.broadcast);
            }
            else
            {
                // For /32, the IP itself is the only host
                info.network   = ip;
                info.broadcast = ip;
                info.hostMin   = ip;
                info.hostMax   = ip;
            }

            return info;
        }

        bool IPCalculator::isValidInput (const std::string &input)
        {
            std::regex pattern (R"((\d{1,3}\.){3}\d{1,3}(/\d{1,2})?)");
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

        std::vector<int> IPCalculator::parseIP (const std::string &ip)
        {
            std::vector<int> octets;
            std::stringstream ss (ip);
            std::string octet;
            while (std::getline (ss, octet, '.'))
            {
                octets.push_back (std::stoi (octet));
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

        std::string IPCalculator::getNetworkAddress (const std::string &ip, const std::string &subnetMask)
        {
            auto ipOctets   = parseIP (ip);
            auto maskOctets = parseIP (subnetMask);
            std::string network;
            for (int i = 0; i < 4; ++i)
            {
                network += std::to_string (ipOctets[i] & maskOctets[i]) + (i < 3 ? "." : "");
            }
            return network;
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

        std::string IPCalculator::intToIP (const std::vector<int> &octets)
        {
            std::stringstream ss;
            for (int i = 0; i < 4; ++i)
            {
                ss << octets[i] << (i < 3 ? "." : "");
            }
            return ss.str ();
        }

        struct IPCalcData : public TableFunctionData
        {
            string ip;
        };

        struct IPCalcLocalState : public LocalTableFunctionState
        {
            std::atomic_bool done{ false };
        };

        unique_ptr<FunctionData> IPCalcFunc::Bind (ClientContext &context, TableFunctionBindInput &input, vector<LogicalType> &return_types, vector<string> &names)
        {
            auto bind_data = make_uniq<IPCalcData> ();
            bind_data->ip  = StringValue::Get (input.inputs[0]);

            // 0. address
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("address");

            // 1. netmask
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("netmask");

            // 2. wildcard
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("wildcard");

            // 3. network
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("network");

            // 4. hostMin
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("hostMin");

            // 5. hostMax
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("hostMax");

            // 6. broadcast
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("broadcast");

            // 7. hostsPerNet
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("hostsPerNet");

            // 8. ipClass
            return_types.emplace_back (LogicalType::VARCHAR);
            names.emplace_back ("ipClass");

            return std::move (bind_data);
        }

        unique_ptr<LocalTableFunctionState> IPCalcFunc::InitLocal (ExecutionContext &context, TableFunctionInitInput &input, GlobalTableFunctionState *global_state_p)
        {
            return make_uniq<IPCalcLocalState> ();
        }

        unique_ptr<GlobalTableFunctionState> IPCalcFunc::InitGlobal (ClientContext &context, TableFunctionInitInput &input)
        {
            return nullptr;
        }

        void IPCalcFunc::Scan (ClientContext &context, TableFunctionInput &data_p, DataChunk &output)
        {
            // Check done
            if (((IPCalcLocalState &)*data_p.local_state).done)
            {
                return;
            }

            auto &data = data_p.bind_data->Cast<IPCalcData> ();

            IPInfo info = IPCalculator::calculate (data.ip);

            output.SetCardinality (1);
            output.data[0].SetValue (0, info.address);
            output.data[1].SetValue (0, info.netmask);
            output.data[2].SetValue (0, info.wildcard);
            output.data[3].SetValue (0, info.network);
            output.data[4].SetValue (0, info.hostMin);
            output.data[5].SetValue (0, info.hostMax);
            output.data[6].SetValue (0, info.broadcast);
            output.data[7].SetValue (0, info.hostsPerNet);
            output.data[8].SetValue (0, info.ipClass);
            // Set done
            auto &local_state = (IPCalcLocalState &)*data_p.local_state;
            local_state.done  = true;
        }

        // Function to extract the information from an IP address
        // void IPCalcFunction (DataChunk &args, ExpressionState &state, Vector &result)
        // {
        //     // Extract the input from the arguments
        //     auto &input_vector = args.data[0];
        //     auto result_data   = FlatVector::GetData<string_t> (result);

        //     for (idx_t i = 0; i < args.size (); i++)
        //     {
        //         auto input = input_vector.GetValue (i).ToString ();

        //         try
        //         {
        //             // Get IP information
        //             IPInfo info = IPCalculator::calculate (input);

        //             // Format the result as a string
        //             std::stringstream ss;
        //             ss << "Address: " << info.address << ", "
        //                << "Netmask: " << info.netmask << ", "
        //                << "Wildcard: " << info.wildcard << ", "
        //                << "Network: " << info.network << ", "
        //                << "HostMin: " << info.hostMin << ", "
        //                << "HostMax: " << info.hostMax << ", "
        //                << "Broadcast: " << info.broadcast << ", "
        //                << "Hosts/Net: " << info.hostsPerNet << ", "
        //                << "Class: " << info.ipClass;

        //             result_data[i] = ss.str ();
        //         }
        //         catch (const std::exception &e)
        //         {
        //             result_data[i] = "Error ipcalc: " + std::string (e.what ());
        //         }
        //     }
        // }
    } // namespace netquack
} // namespace duckdb
