#include "property.hpp"

auto Property::contains(const std::size_t pos) const -> bool {
    if (this->start_ == this->end_) { return pos == this->start_; }
    return pos >= this->start_ && pos < this->end_;
}

auto Property::overlaps(const std::size_t start, const std::size_t end) const -> bool {
    if (this->start_ == this->end_) { return this->start_ >= start && this->start_ < end; }
    if (start == end) { return start >= this->start_ && start < this->end_; }
    // clang-format off
    return this->start_ < end && this->end_ > start;
    // clang-format on
}
