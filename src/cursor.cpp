#include "cursor.hpp"

#include <algorithm>

#include "util.hpp"

void Cursor::up(const Document& doc, const std::size_t n) {
    this->pos_.row_ = util::math::sub_sat(this->pos_.row_, n);
    this->pos_.col_ = util::utf8::idx_to_byte(doc.line(this->pos_.row_), this->pref_col_);
}

void Cursor::down(const Document& doc, const std::size_t n) {
    this->pos_.row_ = std::min(this->pos_.row_ + n, util::math::sub_sat(doc.line_count(), static_cast<std::size_t>(1)));
    this->pos_.col_ = util::utf8::idx_to_byte(doc.line(this->pos_.row_), this->pref_col_);
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
            const auto len = util::utf8::len(line[this->pos_.col_]);
            if (this->pos_.col_ + len > line.size()) { break; }
            this->pos_.col_ += len;
        } else { break; }
    }

    this->update_pref_col(doc);
}

void Cursor::update_pref_col(const Document& doc) {
    if (this->pos_.row_ < doc.line_count()) {
        this->pref_col_ = util::utf8::byte_to_idx(doc.line(this->pos_.row_), this->pos_.col_);
    } else { this->pref_col_ = 0; }
}

