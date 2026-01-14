#ifndef UTF8_HPP_
#define UTF8_HPP_

#include <cstddef>
#include <string_view>

namespace utf8 {
    /// Returns the length of a UTF-8 character.
    std::size_t len(unsigned char ch);

    /// Decodes a UTF-8 character into a codepoint.
    std::size_t decode(std::string_view str);

    /// Encodes a UTF-8 codepoint into bytes.
    void encode(std::string& out, std::size_t codepoint);

    /// Converts a byte index to a logical index.
    std::size_t byte_to_idx(std::string_view line, std::size_t byte, std::size_t tab_width);

    /// Converts a logical index to a byte index.
    std::size_t idx_to_byte(std::string_view line, std::size_t idx, std::size_t tab_width);

    /// Returns the width of a character on the terminal.
    std::size_t char_width(std::string_view ch, std::size_t idx, std::size_t tab_width);

    /// Returns the width of a string on the terminal.
    std::size_t str_width(std::string_view str, std::size_t idx, std::size_t tab_width);
} // namespace utf8

#endif
