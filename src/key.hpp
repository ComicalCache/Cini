#ifndef KEY_HPP_
#define KEY_HPP_

#include <optional>
#include <string_view>
#include <unordered_map>

/// Input key. Keys are normalized.
struct Key {
    friend struct std::hash<Key>;

private:
    /// Unicode codepoint.
    std::size_t code_;
    /// Modifier key bitfield.
    std::size_t mod_;

public:
    /// Parses a key from an ANSI sequence.
    static auto try_parse_ansi(std::string_view buff) -> std::pair<std::optional<Key>, std::size_t>;
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
