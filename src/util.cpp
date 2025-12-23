#include "util.hpp"

#include <cstddef>
#include <cwchar>
#include <fstream>
#include <sstream>

namespace util {
    std::optional<std::string> read_file(const std::filesystem::path& path) {
        std::ifstream file(path);

        if (!file.is_open()) { return std::nullopt; }

        std::stringstream buff{};
        buff << file.rdbuf();

        return buff.str();
    }

    std::size_t char_width(const std::string_view ch, const std::size_t x) {
        if (ch == "\t") { return 4 - x % 4; }
        if (ch == "\r") { return 1; }
        if (ch == "\n") { return 1; }

        return std::max(0, wcwidth(static_cast<wchar_t>(utf8_decode(ch))));
    }

    std::size_t utf8_len(const unsigned char ch) {
        if ((ch & 0x80) == 0) { return 1; }
        if ((ch & 0xE0) == 0xC0) { return 2; }
        if ((ch & 0xF0) == 0xE0) { return 3; }
        if ((ch & 0xF8) == 0xF0) { return 4; }

        // Fallback.
        return 1;
    }

    std::size_t utf8_decode(const std::string_view str) {
        if (str.empty()) { return 0; }

        const auto ch = static_cast<unsigned char>(str[0]);
        const auto len = utf8_len(ch);

        std::size_t code = 0;
        switch (len) {
            case 1: return ch;
            case 2: code |= ch & 0x1F;
            case 3: code |= ch & 0x0F;
            case 4: code |= ch & 0x07;
            default: break;
        }

        // Continuation bytes (0b10xxxxxx).
        for (std::size_t idx = 1; idx < len; ++idx) {
            const auto data = static_cast<unsigned char>(str[idx]) & 0x3F;
            code = data | code << 6;
        }

        return code;
    }

    void utf8_encode(std::stringstream& ss, const std::size_t codepoint) {
        // Based on https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5.

        if (codepoint <= 0x7F) { // ASCII.
            ss << static_cast<char>(codepoint);
        } else if (codepoint <= 0x07FF) { // 2-byte unicode.
            auto data = codepoint >> 6 & 0x1F;
            ss << static_cast<char>(data | 0xC0);
            data = codepoint & 0x3F;
            ss << static_cast<char>(data | 0x80);
        } else if (codepoint <= 0xFFFF) { // 3-byte unicode.
            auto data = codepoint >> 12 & 0x0F;
            ss << static_cast<char>(data | 0xE0);
            data = codepoint >> 6 & 0x3F;
            ss << static_cast<char>(data | 0x80);
            data = codepoint & 0x3F;
            ss << static_cast<char>(data | 0x80);
        } else if (codepoint <= 0x10FFFF) { // 4-byte unicode.
            auto data = codepoint >> 18 & 0x7;
            ss << static_cast<char>(data | 0xF0);
            data = codepoint >> 12 & 0x3F;
            ss << static_cast<char>(data | 0x80);
            data = codepoint >> 6 & 0x3F;
            ss << static_cast<char>(data | 0x80);
            data = codepoint & 0x3F;
            ss << static_cast<char>(data | 0x80);
        } else { // Error.
            ss << "�";
        }
    }
} // namespace util
