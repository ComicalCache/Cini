#include "direction_binding.hpp"

#include <sol/table.hpp>

#include "../types/direction.hpp"

void DirectionBinding::init_bridge(sol::table& core) {
    // clang-format off
        core.new_enum("Direction",
            "Left", Direction::LEFT,
            "Right", Direction::RIGHT,
            "Up", Direction::UP,
            "Down", Direction::DOWN);
    // clang-format on
}
