// Copyright 2025 Arash Hatami

#include "extract_host.hpp"
#include "../utils/url_helpers.hpp"

namespace duckdb
{
    void ExtractHostFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto input = input_vector.GetValue (i).ToString ();
            std::transform (input.begin (), input.end (), input.begin (), ::tolower);

            try
            {
                auto host      = netquack::ExtractHost (input);
                result_data[i] = StringVector::AddString (result, host);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting host: " + std::string (e.what ()));
            }
        }
    }

    namespace netquack
    {
        std::string ExtractHost (const std::string &input)
        {
            if (input.empty())
                return "";

            const char* data = input.data();
            size_t size = input.size();

            std::string_view host = getURLHost(data, size);
            
            return std::string(host);
        }
    } // namespace netquack
} // namespace duckdb
