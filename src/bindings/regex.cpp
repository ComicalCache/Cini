#include "regex_binding.hpp"

#include <sol/table.hpp>

#include "../regex.hpp"

void RegexBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Regex>("Regex",
        /* Functions */
        sol::call_constructor, sol::constructors<Regex(std::string_view)>(),
        "search", [](const Regex& self, const std::string_view text) -> std::vector<RegexMatch> {
            return self.search(text);
        });
    // clang-format on
}
