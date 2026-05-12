#include "bindings.hpp"

#include <utility>

#include <sol/table.hpp>

#include "../input/key_event.hpp"

void KeyBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<KeyEvent>("KeyEvent",
        /* Functions. */
        "to_string", &KeyEvent::to_string,
        "normalize", [](const std::string_view str) -> std::string {
            if (KeyEvent key{0, std::to_underlying(KeyEvent::ModKey::NONE)}; KeyEvent::try_parse_string(str, key)) {
                return key.to_string();
            }

            return std::string(str);
        });
    // clang-format on
}
