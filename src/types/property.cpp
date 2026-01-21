#include "property.hpp"

auto Property::contains(const std::size_t pos) const -> bool { return pos >= this->start_ && pos < this->end_; }

auto Property::overlaps(const std::size_t start, const std::size_t end) const -> bool {
    // clang-format off
    return this->start_ < end && this->end_ > start;
    // clang-format on
}
