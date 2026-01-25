#include "cell.hpp"

#include <algorithm>

#include "../types/face.hpp"

Cell::Cell(const unsigned char ch, const Rgb fg, const Rgb bg) : fg_{fg}, bg_{bg} { this->set_char(ch); }

Cell::Cell(const unsigned char ch, const Face face) {
    this->set_char(ch);
    if (face.fg_) { this->fg_ = *face.fg_; }
    if (face.bg_) { this->bg_ = *face.bg_; }
}

Cell::Cell(const std::string_view str, const Rgb fg, const Rgb bg) : fg_{fg}, bg_{bg} { this->set_utf8(str); }

Cell::Cell(const std::string_view str, const Face face) {
    this->set_utf8(str);
    if (face.fg_) { this->fg_ = *face.fg_; }
    if (face.bg_) { this->bg_ = *face.bg_; }
}

void Cell::set_char(const unsigned char ch) {
    this->data_[0] = ch;
    this->data_[1] = 0;
    this->len_ = 1;
}

void Cell::set_utf8(const std::string_view str) {
    const auto n = std::min(str.size(), static_cast<std::size_t>(4));
    std::memcpy(this->data_.data(), str.data(), n);
    data_[n] = 0;
    len_ = static_cast<uint8_t>(n);
}

void Cell::set_face(const Face face) {
    if (face.fg_) { this->fg_ = *face.fg_; }
    if (face.bg_) { this->bg_ = *face.bg_; }
}

auto Cell::operator==(const Cell& rhs) const -> bool {
    const auto equal_data = std::ranges::equal(
        this->data_.data(), this->data_.data() + this->len_, rhs.data_.data(), rhs.data_.data() + rhs.len_);
    return equal_data && this->fg_ == rhs.fg_ && this->bg_ == rhs.bg_;
}

auto Cell::operator!=(const Cell& rhs) const -> bool { return !(*this == rhs); }
