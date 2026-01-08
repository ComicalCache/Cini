#ifndef CELL_HPP_
#define CELL_HPP_

#include "types/rgb.hpp"

struct Face;

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
    explicit Cell(unsigned char ch, Rgb fg = {255, 255, 255}, Rgb bg = {0, 0, 0});
    Cell(unsigned char ch, Face face);
    explicit Cell(std::string_view str, Rgb fg = {255, 255, 255}, Rgb bg = {0, 0, 0});
    Cell(std::string_view str, Face face);

    /// Sets the cell to an ASCII character.
    void set_char(unsigned char ch);
    /// Sets the cell to a Unicode character.
    void set_utf8(std::string_view str);
    /// Sets the cell to a Face's colors.
    void set_face(Face face);

    bool operator==(const Cell& rhs) const;
    bool operator!=(const Cell& rhs) const;
};

#endif
