#ifndef KEY_HPP_
#define KEY_HPP_

#include <string>
#include <unordered_map>

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

    /// Map string representation to enum value.
    extern std::unordered_map<std::string_view, std::size_t> special_map;
}

/// Input key. Keys are normalized.
struct Key {
    friend struct std::hash<Key>;

private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier keys.
    key::Mod mod_;

public:
    /// Parses a key from an ANSI sequence.
    static std::optional<Key> try_parse_ansi(std::string& buff);
    /// Parses a key from its string representation. Returns true if parsed successfully, false otherwise.
    static bool try_parse_string(std::string_view buff, Key& out);

    Key(std::size_t code, key::Mod mod);

    /// Creates a string representation of a key.
    [[nodiscard]] std::string to_string() const;

    bool operator==(const Key& rhs) const;
    bool operator!=(const Key& rhs) const;
};

// Make key hashable to the Keymap.
template<>
struct std::hash<Key> {
    std::size_t operator()(const Key& k) const noexcept;
};

#endif
