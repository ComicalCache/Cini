#ifndef KEY_SPECIAL_HPP_
#define KEY_SPECIAL_HPP_

#include <cstdint>

/// Special keys that are not characters.
///
/// Some keys have legacy behavior (backspace) or aren't "letters" and need special attention.
///
/// These keys are situated outside the Unicode range.
enum struct KeySpecial : std::uint32_t {
    NONE = 0,
    BACKSPACE = 127,

    ARROW_UP = 0x11000,
    ARROW_DOWN = 0x11001,
    ARROW_LEFT = 0x11002,
    ARROW_RIGHT = 0x11003,
    ENTER = 0x11004,
    TAB = 0x11005,
    INSERT = 0x11006,
    DELETE = 0x11007,
    ESCAPE = 0x11008,
};

#endif
