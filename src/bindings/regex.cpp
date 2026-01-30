#include "../regex.hpp"

#include <sol/table.hpp>

void Regex::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Regex>("Regex",
        /* Functions */
        sol::call_constructor, sol::constructors<Regex(std::string_view)>(),
        "search", [](const Regex& self, const std::string_view text) -> std::vector<RegexMatch> { return self.search(text); });
    // clang-format on
}
