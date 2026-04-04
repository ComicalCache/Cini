#ifndef UTF8_HPP_
#define UTF8_HPP_

#include <cstddef>
#include <string_view>

namespace utf8 {
    /// Returns the length of a UTF-8 character.
    [[nodiscard]]
    auto len(unsigned char ch) -> std::size_t;

    /// Decodes a UTF-8 character into a codepoint.
    [[nodiscard]]
    auto decode(std::string_view str) -> std::size_t;
    /// Encodes a UTF-8 codepoint into bytes.
    void encode(std::string& out, std::size_t codepoint);

    /// Converts a byte index to a logical index.
    [[nodiscard]]
    auto byte_to_idx(std::string_view str, std::size_t byte, std::size_t tab_width) -> std::size_t;
    /// Converts a logical index to a byte index.
    [[nodiscard]]
    auto idx_to_byte(std::string_view str, std::size_t idx, std::size_t tab_width) -> std::size_t;

    /// Returns the width of a character on the terminal.
    [[nodiscard]]
    auto char_width(std::string_view ch, std::size_t tab_offset, std::size_t tab_width) -> std::size_t;
    /// Returns the width of a string on the terminal.
    [[nodiscard]]
    auto str_width(std::string_view str, std::size_t tab_offset, std::size_t tab_width) -> std::size_t;
} // namespace utf8

#endif
