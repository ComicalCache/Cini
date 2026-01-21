#ifndef KEY_HPP_
#define KEY_HPP_

#include <sol/sol.hpp>

/// Input key. Keys are normalized.
struct Key {
    friend struct std::hash<Key>;

private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier key bitfield.
    std::size_t mod_;

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    /// Parses a key from an ANSI sequence.
    static auto try_parse_ansi(std::string& buff) -> std::optional<Key>;
    /// Parses a key from its string representation. Returns true if parsed successfully, false otherwise.
    static auto try_parse_string(std::string_view buff, Key& out) -> bool;

    Key(std::size_t code, std::size_t mod);

    /// Creates the string representation of a key.
    [[nodiscard]]
    auto to_string() const -> std::string;

    auto operator==(const Key& rhs) const -> bool;
    auto operator!=(const Key& rhs) const -> bool;
};

namespace key {
    /// Map string representation to enum value.
    extern std::unordered_map<std::string_view, std::size_t> special_map;
} // namespace key

#endif
