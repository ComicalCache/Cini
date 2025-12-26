#include "cell.hpp"

#include <algorithm>

Cell::Cell(const std::string_view str) {
    this->set_utf8(str);
}

void Cell::set_char(const unsigned char ch) {
    this->data_[0] = ch;
    this->data_[1] = 0;
    this->len_ = 1;
}

void Cell::set_utf8(const std::string_view str) {
    const auto n = std::min(str.size(), static_cast<std::size_t>(4));
    std::memcpy(this->data_, str.data(), n);
    data_[n] = 0;
    len_ = static_cast<uint8_t>(n);
}

bool Cell::operator==(const Cell& rhs) const {
    const auto equal_data = std::ranges::equal(this->data_, this->data_ + this->len_, rhs.data_, rhs.data_ + rhs.len_);
    return equal_data && this->fg_ == rhs.fg_ && this->bg_ == rhs.bg_;
}

bool Cell::operator!=(const Cell& rhs) const { return !(*this == rhs); }
