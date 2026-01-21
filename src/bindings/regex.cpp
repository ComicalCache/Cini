#include "../regex.hpp"

void Regex::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Regex>("Regex",
        sol::call_constructor, sol::constructors<Regex(std::string_view)>(),
        /// Searches a text and returns all matches.
        "search", [](const Regex& self, const std::string_view text) { return self.search(text); });
    // clang-format on
}
