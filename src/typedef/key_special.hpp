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

    // Outside unicode range.
    ARROW_UP = 0x110000,
    ARROW_DOWN = 0x110001,
    ARROW_LEFT = 0x110002,
    ARROW_RIGHT = 0x110003,
    ENTER = 0x110004,
    TAB = 0x110005,
    INSERT = 0x110006,
    DELETE = 0x110007,
    ESCAPE = 0x110008,
};

#endif
