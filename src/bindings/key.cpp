#include "key_binding.hpp"

#include <utility>

#include <sol/table.hpp>

#include "../key.hpp"
#include "../types/key_mod.hpp"

void KeyBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Key>("Key",
        /* Functions. */
        "to_string", &Key::to_string,
        "normalize", [](const std::string_view str) -> std::string {
            if (Key key{0, std::to_underlying(KeyMod::NONE)}; Key::try_parse_string(str, key)) {
                return key.to_string();
            }

            return std::string(str);
        });
    // clang-format on
}
