#include "ansi.hpp"

#include <utility>

#include "../types/rgb.hpp"
#include "assert.hpp"

namespace ansi {
    void enable_kitty_protocol(std::string& buff) { buff.append("\x1B[>1u"); }
    void disable_kitty_protocol(std::string& buff) { buff.append("\x1B[<u"); }

    void move_to(std::string& buff, const std::uint16_t row, const std::uint16_t col) {
        ASSERT_DEBUG // NOLINT(readability-simplify-boolean-expr)
            (row > 0 && col > 0, "Coordinates must be inside screen space.");

        // Five since max { uint16_t } = 65535.
        constexpr auto num_len{5};

        buff.append("\x1B[");

        std::array<char, num_len> row_str{};
        if (auto [ptr, ec] = std::to_chars(row_str.data(), row_str.data() + num_len, row); ec == std::errc()) {
            buff.append(std::string_view(row_str.data(), ptr - row_str.data()));
        } else {
            std::unreachable();
        }
        buff.push_back(';');

        std::array<char, num_len> col_str{};
        if (auto [ptr, ec] = std::to_chars(col_str.data(), col_str.data() + num_len, col); ec == std::errc()) {
            buff.append(std::string_view(col_str.data(), ptr - col_str.data()));
        } else {
            std::unreachable();
        }
        buff.push_back('H');
    }

    void rgb(std::string& buff, const Rgb rgb, const bool foreground) {
        // Five since max { uint16_t } = 65535.
        constexpr auto num_len{5};

        buff.append("\x1B[");
        if (foreground) {
            buff.append("38;2;");
        } else {
            buff.append("48;2;");
        }

        std::array<char, num_len> r_str{};
        if (auto [ptr, ec] = std::to_chars(r_str.data(), r_str.data() + num_len, rgb.r_); ec == std::errc()) {
            buff.append(std::string_view(r_str.data(), ptr - r_str.data()));
        } else {
            std::unreachable();
        }
        buff.push_back(';');

        std::array<char, num_len> g_str{};
        if (auto [ptr, ec] = std::to_chars(g_str.data(), g_str.data() + num_len, rgb.g_); ec == std::errc()) {
            buff.append(std::string_view(g_str.data(), ptr - g_str.data()));
        } else {
            std::unreachable();
        }
        buff.push_back(';');

        std::array<char, num_len> b_str{};
        if (auto [ptr, ec] = std::to_chars(b_str.data(), b_str.data() + num_len, rgb.b_); ec == std::errc()) {
            buff.append(std::string_view(b_str.data(), ptr - b_str.data()));
        } else {
            std::unreachable();
        }

        buff.push_back('m');
    }

    void cursor(std::string& buff, const CursorStyle style) {
        if (style == CursorStyle::HIDDEN) {
            hide_cursor(buff);
            return;
        }

        buff.append("\x1b[");

        char num{};
        // char num[1]{};
        if (auto [ptr, ec] = std::to_chars(&num, &num + 1, std::to_underlying(style)); ec == std::errc()) {
            buff.push_back(num);
        } else {
            std::unreachable();
        }

        buff.append(" q");
    }

    void reset_style(std::string& buff) { buff.append("\x1b[0m"); }

    void hide_cursor(std::string& buff) { buff.append("\x1B[?25l"); }
    void show_cursor(std::string& buff) { buff.append("\x1B[?25h"); }

    void alt_screen(std::string& buff) { buff.append("\x1B[?1049h"); }
    void main_screen(std::string& buff) { buff.append("\x1B[?1049l"); }

    void clear(std::string& buff) { buff.append("\x1B[2J"); }

    void bold(std::string& buff, const bool set) { buff.append(set ? "\x1B[1m" : "\x1B[22m"); }
    void italic(std::string& buff, const bool set) { buff.append(set ? "\x1B[3m" : "\x1B[23m"); }
    void underline(std::string& buff, const bool set) { buff.append(set ? "\x1B[4m" : "\x1B[24m"); }
    void strikethrough(std::string& buff, const bool set) { buff.append(set ? "\x1B[9m" : "\x1B[29m"); }
} // namespace ansi
