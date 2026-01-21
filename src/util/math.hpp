#ifndef MATH_HPP_
#define MATH_HPP_

#include <limits>
#include <type_traits>

namespace math {
    // FIXME: remove this function when upgrading to C++26 in favor if std::add_sat.
    /// Saturating addition.
    template<typename T>
    constexpr auto add_sat(T x, T y) noexcept -> T {
        T res{};
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
    constexpr auto sub_sat(T x, T y) noexcept -> T {
        T res{};
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
} // namespace math

#endif
