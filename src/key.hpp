#ifndef KEY_HPP_
#define KEY_HPP_

#include <sol/sol.hpp>

#include "types/key_mod.hpp"

/// Input key. Keys are normalized.
struct Key {
    friend struct std::hash<Key>;

private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier keys.
    KeyMod mod_;

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    /// Parses a key from an ANSI sequence.
    static std::optional<Key> try_parse_ansi(std::string& buff);
    /// Parses a key from its string representation. Returns true if parsed successfully, false otherwise.
    static bool try_parse_string(std::string_view buff, Key& out);

    Key(std::size_t code, KeyMod mod);

    /// Creates the string representation of a key.
    [[nodiscard]]
    std::string to_string() const;

    bool operator==(const Key& rhs) const;
    bool operator!=(const Key& rhs) const;
};

namespace key {
    /// Map string representation to enum value.
    extern std::unordered_map<std::string_view, std::size_t> special_map;
} // namespace key

#endif
