#include "bindings.hpp"

#include <clip.h>
#include <sol/table.hpp>

void UtilBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<UtilBinding>("Util",
        "get_system_clipboard", [] -> std::string {
            std::string s{};
            clip::get_text(s);
            return s;
        },
        "set_system_clipboard", [](const std::string& str) -> void { clip::set_text(str); });
    // clang-format on
}
