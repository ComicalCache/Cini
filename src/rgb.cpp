#include "rgb.hpp"

void Rgb::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Rgb>("Rgb",
        // Functions.
        sol::call_constructor, sol::constructors<Rgb(uint8_t, uint8_t, uint8_t)>());
    // clang-format on
}

bool Rgb::operator==(const Rgb& rhs) const { return this->r_ == rhs.r_ && this->g_ == rhs.g_ && this->b_ == rhs.b_; }
bool Rgb::operator!=(const Rgb& rhs) const { return !(*this == rhs); }
