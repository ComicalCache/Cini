#include "regex_match_binding.hpp"

#include <sol/table.hpp>

#include "../types/regex_match.hpp"

void RegexMatchBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<RegexMatch>("RegexMatch",
        /* Properties. */
        "start", sol::readonly(&RegexMatch::start_),
        "stop", sol::readonly(&RegexMatch::end_));
    // clang-format on
}
