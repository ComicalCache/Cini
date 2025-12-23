#include "rgb.hpp"

bool Rgb::operator==(const Rgb& rhs) const { return this->r_ == rhs.r_ && this->g_ == rhs.g_ && this->b_ == rhs.b_; }
bool Rgb::operator!=(const Rgb& rhs) const { return !(*this == rhs); }
