#include "rgb_binding.hpp"

#include <sol/table.hpp>

#include "../types/rgb.hpp"

void RgbBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Rgb>("Rgb",
        /* Functions. */
        sol::call_constructor, sol::constructors<Rgb(uint8_t, uint8_t, uint8_t)>());
    // clang-format on
}
