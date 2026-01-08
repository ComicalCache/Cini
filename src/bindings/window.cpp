#include "../window.hpp"

#include <sol/sol.hpp>

#include "../types/direction.hpp"

void Window::init_bridge(sol::table& core) {
    // clang-format off
    core.new_enum("Direction",
        "Left", Direction::LEFT,
        "Right", Direction::RIGHT,
        "Up", Direction::UP,
        "Down", Direction::DOWN);
    // clang-format on
}
