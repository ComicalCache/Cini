#include "utf8.hpp"

#include <string>

#include "assert.hpp"

namespace utf8 {
    auto len(const unsigned char ch) -> std::size_t {
        if ((ch & 0x80) == 0) { return 1; }
        if ((ch & 0xE0) == 0xC0) { return 2; }
        if ((ch & 0xF0) == 0xE0) { return 3; }
        if ((ch & 0xF8) == 0xF0) { return 4; }

        // Fallback.
        return 1;
    }

    auto decode(const std::string_view str) -> std::size_t {
        if (str.empty()) { return 0; }

        const auto ch = static_cast<unsigned char>(str[0]);
        const auto len = utf8::len(ch);

        auto code{0UZ};
        switch (len) {
            case 1: return ch;
            case 2: code |= ch & 0x1F;
            case 3: code |= ch & 0x0F;
            case 4: code |= ch & 0x07;
            default: break;
        }

        // Return error codepoint.
        if (str.size() < len) { return 0xFFFD; }

        // Continuation bytes (0b10xxxxxx).
        for (auto idx{1UZ}; idx < len; idx += 1) {
            const auto data = static_cast<unsigned char>(str[idx]) & 0x3F;
            code = data | code << 6;
        }

        return code;
    }

    void encode(std::string& out, const std::size_t codepoint) {
        // Based on https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5.

        if (codepoint <= 0x7F) { // ASCII.
            out += static_cast<char>(codepoint);
        } else if (codepoint <= 0x07FF) { // 2-byte unicode.
            auto data = codepoint >> 6 & 0x1F;
            out += static_cast<char>(data | 0xC0);
            data = codepoint & 0x3F;
            out += static_cast<char>(data | 0x80);
        } else if (codepoint <= 0xFFFF) { // 3-byte unicode.
            auto data = codepoint >> 12 & 0x0F;
            out += static_cast<char>(data | 0xE0);
            data = codepoint >> 6 & 0x3F;
            out += static_cast<char>(data | 0x80);
            data = codepoint & 0x3F;
            out += static_cast<char>(data | 0x80);
        } else if (codepoint <= 0x10FFFF) { // 4-byte unicode.
            auto data = codepoint >> 18 & 0x7;
            out += static_cast<char>(data | 0xF0);
            data = codepoint >> 12 & 0x3F;
            out += static_cast<char>(data | 0x80);
            data = codepoint >> 6 & 0x3F;
            out += static_cast<char>(data | 0x80);
            data = codepoint & 0x3F;
            out += static_cast<char>(data | 0x80);
        } else { // Error.
            out += "�";
        }
    }

    auto byte_to_idx(const std::string_view str, const std::size_t byte, const std::size_t tab_width) -> std::size_t {
        ASSERT(byte <= str.size(), "");
        return utf8::str_width(str.substr(0, byte), 0, tab_width);
    }

    auto idx_to_byte(const std::string_view str, const std::size_t idx, const std::size_t tab_width) -> std::size_t {
        auto byte{0UZ};
        auto curr_idx{0UZ};

        while (byte < str.size()) {
            const auto len = utf8::len(str[byte]);
            if (byte + len > str.size()) { break; }
            const auto ch = str.substr(byte, len);
            const auto width = utf8::char_width(ch, curr_idx, tab_width);

            // Overshooting.
            if (curr_idx + width > idx) { return byte; }

            curr_idx += width;
            byte += len;

            // Exact match.
            if (curr_idx == idx) { return byte; }
        }

        // Closest match.
        return byte;
    }

    auto char_width(const std::string_view ch, const std::size_t tab_offset, const std::size_t tab_width)
        -> std::size_t {
        if (ch == "\t") { return tab_width - (tab_offset % tab_width); }

        return std::max(1, wcwidth(static_cast<wchar_t>(utf8::decode(ch))));
    }

    auto str_width(const std::string_view str, const std::size_t tab_offset, const std::size_t tab_width)
        -> std::size_t {
        auto width{0UZ};
        auto byte{0UZ};
        auto offset{tab_offset};

        while (byte < str.size()) {
            const auto len = utf8::len(static_cast<unsigned char>(str[byte]));
            if (byte + len > str.size()) { break; }

            const auto ch = str.substr(byte, len);
            const auto w = utf8::char_width(ch, offset, tab_width);

            width += w;
            offset += w;
            byte += len;
        }

        return width;
    }
} // namespace utf8
