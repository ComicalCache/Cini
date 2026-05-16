#ifndef CELL_HPP_
#define CELL_HPP_

#include <array>
#include <optional>
#include <string_view>

#include "../types/rgb.hpp"

struct Face;

/// A Cell represents one cell in the Display. It stores a four-byte UTF-8 character and corresponding cell color.
///
/// Cell data must be managed through the API of this class and never directly inserted. Failure to do so can result in
/// UB and crashes.
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
    /// Underline color.
    std::optional<Rgb> uc_{};

    bool bold_{false};
    bool italic_{false};
    bool underline_{false};
    bool squiggly_{false};
    bool strikethrough_{false};

public:
    Cell() = default;
    Cell(unsigned char ch, Face face);
    Cell(std::string_view str, Face face);

    /// Sets the Cell to an ASCII character.
    void set_char(unsigned char ch);
    /// Sets the Cell to a Unicode character.
    void set_utf8(std::string_view str);
    /// Sets the Cell's Face.
    void set_face(Face face);

    [[nodiscard]]
    auto operator==(const Cell& rhs) const -> bool;
    [[nodiscard]]
    auto operator!=(const Cell& rhs) const -> bool;
};

#endif
