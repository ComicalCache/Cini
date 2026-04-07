#include "bindings.hpp"

#include <sol/table.hpp>

#include "../types/position.hpp"

void PositionBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Position>("Position",
        /* Properties. */
        "row", &Position::row_,
        "col", &Position::col_);
    // clang-format on
}
