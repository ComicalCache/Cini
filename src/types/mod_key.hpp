#ifndef MOD_KEY_HPP_
#define MOD_KEY_HPP_

#include <cstdint>

/// Modifier keys.
enum struct ModKey : std::uint8_t { NONE = 0, CTRL = 1 << 0, ALT = 1 << 1, SHIFT = 1 << 2, SUPER = 1 << 3 };

#endif
