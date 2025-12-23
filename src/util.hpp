#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace util {
    /// Reads a file and returns it contents on success.
    std::optional<std::string> read_file(const std::filesystem::path& path);

    /// Returns the width of a character on the terminal.
    std::size_t char_width(std::string_view ch, std::size_t x);

    /// Returns the length of a UTF-8 character.
    std::size_t utf8_len(unsigned char ch);

    /// Decodes a UTF-8 character into a codepoint.
    std::size_t utf8_decode(std::string_view str);

    /// Encodes a UTF-8 codepoint into bytes.
    void utf8_encode(std::stringstream& ss, std::size_t codepoint);
} // namespace util

#endif
