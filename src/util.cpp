#include "util.hpp"

#include <fstream>

#include "window.hpp"

namespace util {
    std::optional<std::string> read_file(const std::filesystem::path& path) {
        std::ifstream file(path);

        if (!file.is_open()) { return std::nullopt; }

        file.seekg(0, std::ios::end);
        std::string buffer{};
        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::ptrdiff_t>(buffer.size()));

        return buffer;
    }

    bool write_file(const std::filesystem::path& path, const std::string_view contents,
                    const std::ios_base::openmode mode) {
        std::ofstream file(path, mode);

        if (!file.is_open()) { return false; }
        file.write(contents.data(), static_cast<std::ptrdiff_t>(contents.size()));

        return file.good();
    }

    std::size_t char_width(const std::string_view ch, const std::size_t idx, const std::size_t tab_width) {
        if (ch == "\t") { return tab_width - idx % tab_width; }
        if (ch == "\r") { return 1; }
        if (ch == "\n") { return 1; }

        return std::max(0, wcwidth(static_cast<wchar_t>(utf8::decode(ch))));
    }

    std::size_t str_width(const std::string_view str, const std::size_t idx, const std::size_t tab_width) {
        std::size_t width = 0;
        std::size_t byte = 0;
        std::size_t offset = idx;

        while (byte < str.size()) {
            const auto len = utf8::len(static_cast<unsigned char>(str[byte]));
            if (byte + len > str.size()) { break; }

            const auto ch = str.substr(byte, len);
            const auto w = char_width(ch, offset, tab_width);

            width += w;
            offset += w;
            byte += len;
        }

        return width;
    }

    KeyMod parse_xterm_mod(const std::size_t param) {
        auto mod = KeyMod::NONE;
        const auto bitmap = param - 1;

        if (bitmap & 1) { mod |= KeyMod::SHIFT; }
        if (bitmap & 2) { mod |= KeyMod::ALT; }
        if (bitmap & 4) { mod |= KeyMod::CTRL; }

        return mod;
    }

    std::shared_ptr<Viewport> find_viewport(const std::shared_ptr<Window>& node) {
        if (node->viewport_) { // Leaf.
            return node->viewport_;
        } else { // Node.
            // Always prefer the first child.
            return find_viewport(node->child_1_);
        }
    }
}

namespace util::utf8 {
    std::size_t len(const unsigned char ch) {
        if ((ch & 0x80) == 0) { return 1; }
        if ((ch & 0xE0) == 0xC0) { return 2; }
        if ((ch & 0xF0) == 0xE0) { return 3; }
        if ((ch & 0xF8) == 0xF0) { return 4; }

        // Fallback.
        return 1;
    }

    std::size_t decode(const std::string_view str) {
        if (str.empty()) { return 0; }

        const auto ch = static_cast<unsigned char>(str[0]);
        const auto len = utf8::len(ch);

        std::size_t code = 0;
        switch (len) {
            case 1: return ch;
            case 2: code |= ch & 0x1F;
            case 3: code |= ch & 0x0F;
            case 4: code |= ch & 0x07;
            default: break;
        }

        if (str.size() < len) { return 0xFFFD; }

        // Continuation bytes (0b10xxxxxx).
        for (std::size_t idx = 1; idx < len; idx += 1) {
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

    std::size_t byte_to_idx(const std::string_view line, const std::size_t byte, const std::size_t tab_width) {
        return util::str_width(line.substr(0, std::min(byte, line.size())), 0, tab_width);
    }

    std::size_t idx_to_byte(const std::string_view line, const std::size_t idx, const std::size_t tab_width) {
        std::size_t byte = 0;
        std::size_t curr_idx = 0;

        while (byte < line.size()) {
            const auto len = utf8::len(line[byte]);
            if (byte + len > line.size()) { break; }
            const auto ch = line.substr(byte, len);
            const auto width = char_width(ch, curr_idx, tab_width);

            if (curr_idx + width > idx) { return byte; }

            curr_idx += width;
            byte += len;

            if (curr_idx == idx) { return byte; }
        }

        return byte;
    }
}

namespace util::log {
    void set_status_message(const std::string_view msg) {
        if (util::log::status_massage_handler) { util::log::status_massage_handler(msg); }
    }
}
