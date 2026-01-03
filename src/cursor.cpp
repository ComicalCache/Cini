#include "cursor.hpp"

#include "document.hpp"
#include "util.hpp"

void Cursor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Cursor>("Cursor",
        "row", sol::property([](const Cursor& self) { return self.pos_.row_; }),
        "col", sol::property([](const Cursor& self) { return self.pos_.col_; }),
        "byte", &Cursor::byte);
    // clang-format on
}

void Cursor::up(const Document& doc, const std::size_t n) {
    this->pos_.row_ = util::math::sub_sat(this->pos_.row_, n);

    const auto line = doc.line(this->pos_.row_);
    this->pos_.col_ = util::utf8::idx_to_byte(line, this->pref_col_);

    if (line.ends_with('\n') && this->pos_.col_ == line.size()) { this->pos_.col_ -= 1; }
}

void Cursor::down(const Document& doc, const std::size_t n) {
    this->pos_.row_ = std::min(this->pos_.row_ + n, util::math::sub_sat(doc.line_count(), static_cast<std::size_t>(1)));

    const auto line = doc.line(this->pos_.row_);
    this->pos_.col_ = util::utf8::idx_to_byte(line, this->pref_col_);

    if (line.ends_with('\n') && this->pos_.col_ == line.size()) { this->pos_.col_ -= 1; }
}

void Cursor::left(const Document& doc, const std::size_t n) {
    for (std::size_t i = 0; i < n; i += 1) {
        if (this->pos_.col_ > 0) {
            const auto line = doc.line(this->pos_.row_);

            // Move back one byte, then continue moving back until the start of the utf-8 byte sequence.
            // Continuation bytes start with 10xxxxxx.
            this->pos_.col_ -= 1;
            while (this->pos_.col_ > 0 && (static_cast<unsigned char>(line[this->pos_.col_]) & 0xC0) == 0x80) {
                this->pos_.col_ -= 1;
            }
        } else { break; }
    }

    this->update_pref_col(doc);
}

void Cursor::right(const Document& doc, const std::size_t n) {
    for (std::size_t i = 0; i < n; i += 1) {
        if (const auto line = doc.line(this->pos_.row_); this->pos_.col_ < line.size()) {
            if (line[this->pos_.col_] == '\n') { break; }

            const auto len = util::utf8::len(line[this->pos_.col_]);
            if (this->pos_.col_ + len > line.size()) { break; }
            this->pos_.col_ += len;
        } else { break; }
    }

    this->update_pref_col(doc);
}

std::size_t Cursor::byte(const Document& doc) const {
    std::size_t pos = 0;
    for (std::size_t idx = 0; idx < this->pos_.row_; idx += 1) { pos += doc.line(idx).size(); }
    return pos + this->pos_.col_;
}

void Cursor::update_pref_col(const Document& doc) {
    if (this->pos_.row_ < doc.line_count()) {
        this->pref_col_ = util::utf8::byte_to_idx(doc.line(this->pos_.row_), this->pos_.col_);
    } else { this->pref_col_ = 0; }
}

