// Copyright 2025 Arash Hatami

#include "extract_port.hpp"

#include "../utils/url_helpers.hpp"

namespace duckdb
{
    void ExtractPortFunction (DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &input_vector    = args.data[0];
        auto result_data      = FlatVector::GetData<string_t> (result);
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
                auto port      = netquack::ExtractPort (input);
                result_data[i] = StringVector::AddString (result, port);
            }
            catch (const std::exception &e)
            {
                result_data[i] = StringVector::AddString (result, "Error extracting port: " + std::string (e.what ()));
            }
        }
    }

    namespace netquack
    {
        std::string ExtractPort (const std::string &input)
        {
            if (input.empty ())
            {
                return "";
            }

            const char *data = input.data ();
            size_t size      = input.size ();
            const char *pos  = data;
            const char *end  = pos + size;

            // Skip protocol if present
            if (size >= 2 && *pos == '/' && *(pos + 1) == '/')
            {
                pos += 2;
            }
            else
            {
                const char *scheme_end = data + std::min (size, static_cast<size_t> (16));
                for (++pos; pos < scheme_end; ++pos)
                {
                    if (!isAlphaNumericASCII (*pos))
                    {
                        switch (*pos)
                        {
                        case '.':
                        case '-':
                        case '+':
                            break;
                        default:
                            goto exloop;
                        }
                    }
                }
            exloop:
                if ((scheme_end - pos) > 2 && *pos == ':' && *(pos + 1) == '/' && *(pos + 2) == '/')
                {
                    pos += 3;
                }
                else
                {
                    pos = data;
                }
            }

            // Skip authentication if present (user:pass@)
            const char *at_pos = nullptr;
            for (const char *p = pos; p < end; ++p)
            {
                if (*p == '@')
                {
                    at_pos = p;
                    break;
                }
                if (*p == '/' || *p == '?' || *p == '#')
                {
                    break;
                }
            }

            if (at_pos)
            {
                pos = at_pos + 1; // skip authentication part
            }

            // Handle IPv6 addresses in brackets [address]:port
            if (pos < end && *pos == '[')
            {
                // Find the closing bracket
                for (const char *p = pos + 1; p < end; ++p)
                {
                    if (*p == ']')
                    {
                        pos = p + 1; // Move past the closing bracket
                        break;
                    }
                }
            }

            // Now find the port after the host
            const char *port_start = nullptr;
            for (const char *p = pos; p < end; ++p)
            {
                if (*p == ':')
                {
                    // For IPv6, we're already past the brackets, so any colon is the port
                    // For regular hosts, check if this is a port (followed by digits)
                    const char *next = p + 1;
                    if (next < end && isNumericASCII (*next))
                    {
                        port_start = next;
                        break;
                    }
                }
                else if (*p == '/' || *p == '?' || *p == '#')
                {
                    break;
                }
            }

            if (!port_start)
            {
                return "";
            }

            // Extract port digits
            std::string port;
            for (const char *p = port_start; p < end; ++p)
            {
                if (*p == '/' || *p == '?' || *p == '#')
                {
                    break;
                }
                if (!isNumericASCII (*p))
                {
                    return "";
                }

                port += *p;
            }

            return port;
        }
    } // namespace netquack
} // namespace duckdb
