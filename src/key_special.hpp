#ifndef KEY_SPECIAL_HPP_
#define KEY_SPECIAL_HPP_

#include <cstddef>

/// Special keys that are not characters.
///
/// Some keys have legacy behavior (backspace) or aren't "letters" and need special attention.
///
/// These keys are situated outside the Unicode range.
enum struct KeySpecial : std::size_t {
    NONE      = 0,
    BACKSPACE = 127,

    ARROW_UP = 0x11000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    ENTER,
    TAB,
    INSERT,
    DELETE,
    ESCAPE,
};

#endif
