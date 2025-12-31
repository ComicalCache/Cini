#include "ansi.hpp"

#include "rgb.hpp"

namespace ansi {
    void move_to(std::string& buff, const std::uint16_t row, const std::uint16_t col) {
        assert(row > 0 && col > 0);

        // Five since max { uint16_t } = 65535.
        constexpr auto num_len = 5;

        buff.append("\x1B[");

        char row_str[num_len]{};
        if (auto [ptr, ec] = std::to_chars(row_str, row_str + num_len, row); ec == std::errc()) {
            buff.append(std::string_view(row_str, ptr - row_str));
        } else { std::unreachable(); }
        buff.push_back(';');

        char col_str[num_len]{};
        if (auto [ptr, ec] = std::to_chars(col_str, col_str + num_len, col); ec == std::errc()) {
            buff.append(std::string_view(col_str, ptr - col_str));
        } else { std::unreachable(); }
        buff.push_back('H');
    }

    void rgb(std::string& buff, const Rgb rgb, const bool foreground) {
        // Five since max { uint16_t } = 65535.
        constexpr auto num_len = 5;

        buff.append("\x1B[");
        if (foreground) { buff.append("38;2;"); } else { buff.append("48;2;"); }

        char r_str[num_len]{};
        if (auto [ptr, ec] = std::to_chars(r_str, r_str + num_len, rgb.r_); ec == std::errc()) {
            buff.append(std::string_view(r_str, ptr - r_str));
        } else { std::unreachable(); }
        buff.push_back(';');

        char g_str[num_len]{};
        if (auto [ptr, ec] = std::to_chars(g_str, g_str + num_len, rgb.g_); ec == std::errc()) {
            buff.append(std::string_view(g_str, ptr - g_str));
        } else { std::unreachable(); }
        buff.push_back(';');

        char b_str[num_len]{};
        if (auto [ptr, ec] = std::to_chars(b_str, b_str + num_len, rgb.b_); ec == std::errc()) {
            buff.append(std::string_view(b_str, ptr - b_str));
        } else { std::unreachable(); }

        buff.push_back('m');
    }

    void cursor(std::string& buff, CursorStyle style) {
        if (style == CursorStyle::HIDDEN) {
            hide_cursor(buff);
            return;
        }

        buff.append("\x1b[");

        char num[1]{};
        if (auto [ptr, ec] = std::to_chars(num, num + 1, static_cast<std::size_t>(style)); ec == std::errc()) {
            buff.append(std::string_view(num, ptr - num));
        } else { std::unreachable(); }

        buff.append(" q");
    }

    void reset_style(std::string& buff) { buff.append("\x1b[0m"); }

    void hide_cursor(std::string& buff) { buff.append("\x1B[?25l"); }
    void show_cursor(std::string& buff) { buff.append("\x1B[?25h"); }

    void alt_screen(std::string& buff) { buff.append("\x1B[?1049h"); }
    void main_screen(std::string& buff) { buff.append("\x1B[?1049l"); }

    void clear(std::string& buff) { buff.append("\x1B[2J"); }
}
