#ifndef CELL_HPP_
#define CELL_HPP_

#include <array>

#include "../types/rgb.hpp"

struct Face;

/// Cell of the Display.
struct Cell {
public:
    /// Unicode codepoint data of a character.
    std::array<unsigned char, 5> data_{};
    /// Byte length of the codepoint.
    std::uint8_t len_{0};

    /// Foreground color.
    Rgb fg_{.r_ = 255, .g_ = 255, .b_ = 255};
    /// Background color.
    Rgb bg_{.r_ = 0, .g_ = 0, .b_ = 0};

public:
    Cell() = default;
    explicit Cell(unsigned char ch, Rgb fg = {.r_ = 255, .g_ = 255, .b_ = 255}, Rgb bg = {.r_ = 0, .g_ = 0, .b_ = 0});
    Cell(unsigned char ch, Face face);
    explicit Cell(
        std::string_view str, Rgb fg = {.r_ = 255, .g_ = 255, .b_ = 255}, Rgb bg = {.r_ = 0, .g_ = 0, .b_ = 0});
    Cell(std::string_view str, Face face);

    /// Sets the cell to an ASCII character.
    void set_char(unsigned char ch);
    /// Sets the cell to a Unicode character.
    void set_utf8(std::string_view str);
    /// Sets the cell to a Face's colors.
    void set_face(Face face);

    auto operator==(const Cell& rhs) const -> bool;
    auto operator!=(const Cell& rhs) const -> bool;
};

#endif
