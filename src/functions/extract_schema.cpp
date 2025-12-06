// Copyright 2025 Arash Hatami

#include "extract_schema.hpp"
#include <cstring>

namespace duckdb
{
    void ExtractSchemaFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &input_vector = args.data[0];
        auto result_data   = FlatVector::GetData<string_t> (result);
        auto &result_validity = FlatVector::Validity (result);

        for (idx_t i = 0; i < args.size (); i++)
        {
            auto value = input_vector.GetValue (i);
            if (value.IsNull ())
            {
                result_validity.SetInvalid (i);
                continue;
            }

            auto input = value.ToString ();
            std::transform (input.begin (), input.end (), input.begin (), ::tolower);

            try
            {
                auto schema    = netquack::ExtractSchema (input);
                result_data[i] = StringVector::AddString (result, schema);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting schema: " + std::string (e.what ()));
            }
        }
    }

    namespace netquack
    {
        std::string ExtractSchema (const std::string &input)
        {
            if (input.empty())
                return "";

            const char* data = input.data();
            size_t size = input.size();

            // Check for standard URL schemes with ://
            if (size >= 7 && strncmp(data, "http://", 7) == 0)
                return "http";
            if (size >= 8 && strncmp(data, "https://", 8) == 0)
                return "https";
            if (size >= 6 && strncmp(data, "ftp://", 6) == 0)
                return "ftp";
            if (size >= 8 && strncmp(data, "rsync://", 8) == 0)
                return "rsync";

            // Check for schemes with : but no // (and ensure they're not followed by //)
            if (size >= 7 && strncmp(data, "mailto:", 7) == 0) {
                // Reject mailto:// - only accept mailto: without //
                if (size == 7 || (size > 7 && data[7] != '/'))
                    return size > 7 ? "mailto" : "";  // Empty if just "mailto:"
            }
            if (size >= 4 && strncmp(data, "tel:", 4) == 0) {
                // Reject tel:// - only accept tel: without //
                if (size == 4 || (size > 4 && data[4] != '/'))
                    return size > 4 ? "tel" : "";  // Empty if just "tel:"
            }
            if (size >= 4 && strncmp(data, "sms:", 4) == 0) {
                // Reject sms:// - only accept sms: without //
                if (size == 4 || (size > 4 && data[4] != '/'))
                    return size > 4 ? "sms" : "";  // Empty if just "sms:"
            }

            return "";
        }
    } // namespace netquack
} // namespace duckdb
