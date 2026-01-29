#include "../types/regex_match.hpp"

#include <sol/table.hpp>

void RegexMatch::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<RegexMatch>("RegexMatch",
        /* Properties. */
        /// Start of the match.
        "start", sol::readonly(&RegexMatch::start_),
        /// End of the match.
        "stop", sol::readonly(&RegexMatch::end_));
    // clang-format on
}
