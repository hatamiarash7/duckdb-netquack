// Copyright 2025 Arash Hatami

#pragma once

#include <string_view>
#include <cstring>

namespace duckdb
{
    namespace netquack
    {
        using Pos = const char *;

        inline bool isAlphaNumericASCII(char c)
        {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        inline bool isAlphaASCII(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        inline bool isNumericASCII(char c)
        {
            return (c >= '0' && c <= '9');
        }

        template<char symbol>
        inline Pos find_first_symbols(Pos begin, Pos end)
        {
            for (Pos pos = begin; pos < end; ++pos)
            {
                if (*pos == symbol)
                    return pos;
            }
            return end;
        }

        template<char symbol1, char symbol2>
        inline Pos find_first_symbols(Pos begin, Pos end)
        {
            for (Pos pos = begin; pos < end; ++pos)
            {
                if (*pos == symbol1 || *pos == symbol2)
                    return pos;
            }
            return end;
        }

        template<char symbol>
        inline Pos find_last_symbols_or_null(Pos begin, Pos end)
        {
            for (Pos pos = end - 1; pos >= begin; --pos)
            {
                if (*pos == symbol)
                    return pos;
            }
            return nullptr;
        }

        inline std::string_view getURLScheme(const char * data, size_t size)
        {
            const char * pos = data;
            const char * end = data + size;

            if (isAlphaASCII(*pos))
            {
                for (++pos; pos < end; ++pos)
                {
                    if (!(isAlphaNumericASCII(*pos) || *pos == '+' || *pos == '-' || *pos == '.'))
                    {
                        break;
                    }
                }

                return std::string_view(data, pos - data);
            }

            return {};
        }

        inline std::string_view checkAndReturnHost(const Pos & pos, const Pos & dot_pos, const Pos & start_of_host)
        {
            if (!dot_pos || start_of_host >= pos || pos - dot_pos == 1)
                return std::string_view{};

            auto after_dot = *(dot_pos + 1);
            if (after_dot == ':' || after_dot == '/' || after_dot == '?' || after_dot == '#')
                return std::string_view{};

            return std::string_view(start_of_host, pos - start_of_host);
        }

        inline std::string_view getURLHost(const char * data, size_t size)
        {
            Pos pos = data;
            Pos end = data + size;

            // Handle protocol-relative URLs (//example.com)
            if (size >= 2 && *pos == '/' && *(pos + 1) == '/')
            {
                pos += 2;
            }
            else
            {
                // Parse scheme://host or handle bare hostnames
                Pos scheme_end = data + std::min(size, static_cast<size_t>(16));
                for (++pos; pos < scheme_end; ++pos)
                {
                    if (!isAlphaNumericASCII(*pos))
                    {
                        switch (*pos)
                        {
                        case '.':
                        case '-':
                        case '+':
                            break;
                        case ' ': // restricted symbols
                        case '\t':
                        case '<':
                        case '>':
                        case '%':
                        case '{':
                        case '}':
                        case '|':
                        case '\\':
                        case '^':
                        case '~':
                        case '[':
                        case ']':
                        case ';':
                        case '=':
                        case '&':
                            return std::string_view{};
                        default:
                            goto exloop;
                        }
                    }
                }
        exloop: if ((scheme_end - pos) > 2 && *pos == ':' && *(pos + 1) == '/' && *(pos + 2) == '/')
                    pos += 3;
                else
                    pos = data;
            }

            Pos dot_pos = nullptr;
            const auto * start_of_host = pos;
            for (; pos < end; ++pos)
            {
                switch (*pos)
                {
                case '.':
                    dot_pos = pos;
                    break;
                case ':': // end symbols
                case '/':
                case '?':
                case '#':
                    return checkAndReturnHost(pos, dot_pos, start_of_host);
                case '@': // Handle user:pass@host format
                    start_of_host = pos + 1;
                    break;
                case ' ': // restricted symbols in whole URL
                case '\t':
                case '<':
                case '>':
                case '%':
                case '{':
                case '}':
                case '|':
                case '\\':
                case '^':
                case '~':
                case '[':
                case ']':
                case ';':
                case '=':
                case '&':
                    return std::string_view{};
                }
            }

            return checkAndReturnHost(pos, dot_pos, start_of_host);
        }

    } // namespace netquack
} // namespace duckdb
