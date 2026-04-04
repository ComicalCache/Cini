#ifndef KEY_HPP_
#define KEY_HPP_

#include <optional>
#include <string_view>
#include <unordered_map>

/// Keys abstract input keys by storing them as their Unicode codepoint and key-modifier. Special keys like the arrow
/// keys, which don't have a canonical Unicode codepoint, or legacy keys, like backspace, are mapped to the
/// corresponding SpecialKey enum variant. The modifiers are a bitfield of the corresponding ModKey enum variants.
struct Key {
    friend struct std::hash<Key>;

private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier key bitfield.
    std::size_t mod_;

public:
    /// Parses a Key from an ANSI sequence, returing the parsed Key and how much data was consumed.
    [[nodiscard]]
    static auto try_parse_ansi(std::string_view buff) -> std::pair<std::optional<Key>, std::size_t>;
    /// Parses a Key from its string representation. Returns true if parsed successfully, false otherwise.
    [[nodiscard]]
    static auto try_parse_string(std::string_view buff, Key& out) -> bool;

    Key(std::size_t code, std::size_t mod);

    /// Creates the string representation of a key.
    [[nodiscard]]
    auto to_string() const -> std::string;

    [[nodiscard]]
    auto operator==(const Key& rhs) const -> bool;
    [[nodiscard]]
    auto operator!=(const Key& rhs) const -> bool;
};

namespace key {
    /// Map string representation to enum value.
    extern std::unordered_map<std::string_view, std::size_t> special_map;
} // namespace key

/// Special keys that are not characters. Some keys have legacy behavior (backspace) or aren't "letters" and need
/// special handling.
///
/// These keys are situated outside the Unicode range.
enum struct SpecialKey : std::uint32_t {
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

/// Modifier keys.
enum struct ModKey : std::uint8_t { NONE = 0, CTRL = 1 << 0, ALT = 1 << 1, SHIFT = 1 << 2, SUPER = 1 << 3 };

#endif
