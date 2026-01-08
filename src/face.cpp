#include "face.hpp"

void Face::merge(const Face& other) {
    if (other.fg_) { this->fg_ = other.fg_; }
    if (other.bg_) { this->bg_ = other.bg_; }
}
