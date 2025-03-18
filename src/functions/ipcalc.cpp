#include "ipcalc.hpp"

#include "../utils/ip_utils.hpp"

namespace duckdb
{
    namespace netquack
    {
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
    } // namespace netquack
} // namespace duckdb
