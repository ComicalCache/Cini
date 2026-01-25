#include "../../types/regex_match.hpp"

#include <sol/table.hpp>

void RegexMatch::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<RegexMatch>("RegexMatch",
        /* Properties. */
        /// Start of the match.
        "start", &RegexMatch::start_,
        /// End of the match.
        "stop", &RegexMatch::end_);
    // clang-format on
}
