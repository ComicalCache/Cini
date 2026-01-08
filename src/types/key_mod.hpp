#ifndef KEY_MOD_HPP_
#define KEY_MOD_HPP_

#include <cstddef>

/// Modifier keys.
enum struct KeyMod : std::size_t { NONE = 0, CTRL = 1 << 0, ALT = 1 << 1, SHIFT = 1 << 2 };

inline KeyMod operator|(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
}

inline KeyMod& operator|=(KeyMod& a, const KeyMod b) { return a = a | b; }

inline KeyMod operator&(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<std::size_t>(a) & static_cast<std::size_t>(b));
}

inline KeyMod& operator&=(KeyMod& a, const KeyMod b) { return a = a & b; }

#endif
