#ifndef CELL_HPP_
#define CELL_HPP_

#include <string_view>

#include "rgb.hpp"

/// Cell of the Display.
struct Cell {
public:
    /// Unicode codepoint data of a character.
    unsigned char data_[5]{};
    /// Byte length of the codepoint.
    std::uint8_t len_{0};

    /// Foreground color.
    Rgb fg_{255, 255, 255};
    /// Background color.
    Rgb bg_{0, 0, 0};

public:
    Cell() = default;
    explicit Cell(std::string_view str, Rgb fg = {255, 255, 255}, Rgb bg = {0, 0, 0});

    /// Sets the cell to an ASCII character.
    void set_char(unsigned char ch);
    /// Sets the cell to a Unicode character.
    void set_utf8(std::string_view str);

    bool operator==(const Cell& rhs) const;
    bool operator!=(const Cell& rhs) const;
};

#endif
