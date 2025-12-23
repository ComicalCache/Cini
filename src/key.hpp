#ifndef KEY_HPP_
#define KEY_HPP_

#include <string>

#include "util.hpp"

namespace key {
    /// Modifier keys.
    enum struct Mod : std::size_t { NONE = 0, CTRL = 1 << 0, ALT = 1 << 1, SHIFT = 1 << 2 };

    /// Parses Xterm-style modifiers.
    Mod parse_xterm_mod(std::size_t param);

    inline Mod operator|(Mod a, Mod b) {
        return static_cast<Mod>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
    }

    inline Mod& operator|=(Mod& a, const Mod b) { return a = a | b; }

    inline Mod operator&(Mod a, Mod b) {
        return static_cast<Mod>(static_cast<std::size_t>(a) & static_cast<std::size_t>(b));
    }

    inline Mod& operator&=(Mod& a, const Mod b) { return a = a & b; }

    /// Special keys that are not characters.
    ///
    /// Some keys have legacy behavior (backspace) or aren't "letters" and need special attention.
    ///
    /// These keys are situated outside the Unicode range.
    enum struct Special : std::size_t {
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
} // namespace key

/// Input key.
struct Key {
private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier keys.
    key::Mod mod_;

public:
    Key(std::size_t code, key::Mod mod);

    /// Creates a string representation of a key.
    std::string to_string() const;

    bool operator==(const Key& rhs) const;
    bool operator!=(const Key& rhs) const;

    /// Returns true if parsed successfully, false otherwise.
    static bool try_parse(std::string& buff, Key& out);
};

#endif
