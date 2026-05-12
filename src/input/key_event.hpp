#ifndef KEY_EVENT_HPP_
#define KEY_EVENT_HPP_

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

/// KeyEvents abstract input keys by storing them as their Unicode codepoint and key-modifier. Special keys like the
/// arrow keys, which don't have a canonical Unicode codepoint, or legacy keys, like backspace, are mapped to the
/// corresponding SpecialKey enum variant. The modifiers are a bitfield of the corresponding ModKey enum variants.
struct KeyEvent {
public:
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

private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier key bitfield.
    std::size_t mod_;

public:
    /// Parses a KeyEvent from its string representation. Returns true if parsed successfully, false otherwise.
    [[nodiscard]]
    static auto try_parse_string(std::string_view buff, KeyEvent& out) -> bool;

    KeyEvent(std::size_t code, std::size_t mod);

    /// Creates the string representation of a KeyEvent.
    [[nodiscard]]
    auto to_string() const -> std::string;
};

namespace key_event {
    /// Map string representation to enum value.
    extern std::unordered_map<std::string_view, std::size_t> special_map;
} // namespace key_event

#endif
