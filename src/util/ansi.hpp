#ifndef ANSI_HPP_
#define ANSI_HPP_

#include <string>

struct Rgb;

namespace ansi {
    /// Parses Xterm-style modifiers.
    auto parse_xterm_mod(std::size_t param) -> std::size_t;

    /// Enables the Kitty Keyboard Protocol
    void enable_kitty_protocol(std::string& buff);
    /// Disables the Kitty Keyboard Protocol.
    void disable_kitty_protocol(std::string& buff);

    enum struct CursorStyle : std::uint8_t {
        HIDDEN = 0,
        BLINKING_BLOCK = 1,
        STEADY_BLOCK = 2,
        BLINKING_UNDERLINE = 3,
        STEADY_UNDERLINE = 4,
        BLINKING_BAR = 5,
        STEADY_BAR = 6
    };

    /// Moves the terminal cursor to a row and column (one indexed).
    void move_to(std::string& buff, std::uint16_t row, std::uint16_t col);

    /// Sets the RGB color value for the foreground (default) or background.
    void rgb(std::string& buff, Rgb rgb, bool foreground = true);
    /// Sets the cursor style.
    void cursor(std::string& buff, CursorStyle style);
    /// Resets all style options to the terminal's defaults.
    void reset_style(std::string& buff);

    /// Hides the terminal cursor.
    void hide_cursor(std::string& buff);
    /// Shows the terminal cursor.
    void show_cursor(std::string& buff);

    /// Enables the alternate screen.
    void alt_screen(std::string& buff);
    /// Returns to the main screen.
    void main_screen(std::string& buff);

    /// Clears the terminal.
    void clear(std::string& buff);
} // namespace ansi

#endif
