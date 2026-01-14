#include "property.hpp"

bool Property::contains(const std::size_t pos) const { return pos >= this->start_ && pos < this->end_; }

bool Property::overlaps(const std::size_t start, const std::size_t end) const {
    // clang-format off
    return this->start_ < end && this->end_ > start;
    // clang-format on
}
