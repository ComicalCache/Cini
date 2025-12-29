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
}

namespace util::utf8 {
    /// Returns the length of a UTF-8 character.
    std::size_t len(unsigned char ch);

    /// Decodes a UTF-8 character into a codepoint.
    std::size_t decode(std::string_view str);

    /// Encodes a UTF-8 codepoint into bytes.
    void encode(std::string& out, std::size_t codepoint);

    /// Converts a byte index to a logical index.
    std::size_t byte_to_idx(std::string_view line, std::size_t byte);

    /// Converts a logical index to a byte index.
    std::size_t idx_to_byte(std::string_view line, std::size_t idx);
}

namespace util::math {
    // FIXME: remove this function when upgrading to C++26 in favor if std::add_sat.
    /// Saturating addition.
    template<typename T>
    constexpr T add_sat(T x, T y) noexcept {
        T res;
        #ifdef __GNUC__
        if (__builtin_add_overflow(x, y, &res)) {
            if constexpr (std::is_signed_v<T>) {
                // If x is positive, we overflowed towards MAX.
                // If x is negative, we overflowed towards MIN.
                return x > 0 ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min();
            } else {
                // Unsigned overflow is always towards MAX.
                return std::numeric_limits<T>::max();
            }
        }
        return res;
        #else
        if constexpr (std::is_unsigned_v<T>) {
            if (std::numeric_limits<T>::max() - x < y) return std::numeric_limits<T>::max();
            return x + y;
        } else {
            if (x > 0 && y > 0 && x > std::numeric_limits<T>::max() - y) return std::numeric_limits<T>::max();
            if (x < 0 && y < 0 && x < std::numeric_limits<T>::min() - y) return std::numeric_limits<T>::min();
            return x + y;
        }
        #endif
    }

    // FIXME: remove this function when upgrading to C++26 in favor if std::sub_sat.
    /// Saturating subtraction.
    template<typename T>
    constexpr T sub_sat(T x, T y) noexcept {
        T res;
        #ifdef __GNUC__
        if (__builtin_sub_overflow(x, y, &res)) {
            if constexpr (std::is_signed_v<T>) {
                // x positive, y negative -> x - y > MAX.
                // x negative, y positive -> x - y < MIN.
                return x > 0 ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min();
            } else {
                // Unsigned underflow always goes to 0.
                return std::numeric_limits<T>::min();
            }
        }
        return res;
        #else
        if constexpr (std::is_unsigned_v<T>) {
            if (x < y) return 0;
            return x - y;
        } else {
            if (x > 0 && y < 0 && x > std::numeric_limits<T>::max() + y) return std::numeric_limits<T>::max();
            if (x < 0 && y > 0 && x < std::numeric_limits<T>::min() + y) return std::numeric_limits<T>::min();
            return x - y;
        }
        #endif
    }
}

#endif
