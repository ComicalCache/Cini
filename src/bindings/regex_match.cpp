#include "../types/regex_match.hpp"

#include <sol/table.hpp>

void RegexMatch::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<RegexMatch>("RegexMatch",
        /* Properties. */
        "start", sol::readonly(&RegexMatch::start_),
        "stop", sol::readonly(&RegexMatch::end_));
    // clang-format on
}
