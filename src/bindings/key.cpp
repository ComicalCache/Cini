#include "../key.hpp"

#include <sol/sol.hpp>

void Key::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Key>("Key",
        /* Functions. */
        /// Converts a Key into its string representation.
        "to_string", &Key::to_string,
        /// Normalizes a Key string.
        "normalize", [](const std::string_view str) -> std::string {
            if (Key key{0, KeyMod::NONE}; try_parse_string(str, key)) { return key.to_string(); }
            return std::string(str);
        });
    // clang-format on
}
