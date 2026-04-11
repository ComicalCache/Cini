#include "bindings.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#include <sol/optional.hpp>
#include <sol/table.hpp>

#include "../regex.hpp"

void RegexBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Regex>("Regex",
        /* Functions */
        sol::call_constructor, [](std::string_view pattern)
            -> std::pair<std::optional<Regex>, std::optional<std::string>> {
            try {
                return {Regex(pattern), std::nullopt};
            } catch (const std::runtime_error& err) {
                return {std::nullopt, std::string{err.what()}};
            }
        },
        "search", [](const Regex& self, const std::string_view text) -> std::vector<RegexMatch> {
            return self.search(text);
        });
    // clang-format on
}
