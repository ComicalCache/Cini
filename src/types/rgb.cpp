#include "rgb.hpp"

auto Rgb::operator==(const Rgb& rhs) const -> bool {
    return this->r_ == rhs.r_ && this->g_ == rhs.g_ && this->b_ == rhs.b_;
}
auto Rgb::operator!=(const Rgb& rhs) const -> bool { return !(*this == rhs); }
