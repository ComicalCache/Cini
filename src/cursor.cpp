#include "cursor.hpp"

#include "document.hpp"
#include "util/math.hpp"
#include "util/utf8.hpp"

void Cursor::up(const Document& doc, const std::size_t n) {
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }

    this->pos_.row_ = math::sub_sat(this->pos_.row_, n);
    this->pos_.col_ = 0;

    const auto row = this->pos_.row_;
    std::size_t col = 0;
    while (this->pos_.row_ == row) {
        const auto line = doc.line(this->pos_.row_);
        if (const auto len = line.ends_with('\n') ? line.size() - 1 : line.size(); this->pos_.col_ >= len) { break; }

        std::size_t atom_width = 0;
        const auto point = this->point(doc);

        if (const auto* const property = doc.get_raw_text_property(point, "replacement")) {
            atom_width = utf8::str_width(property->value_.as<std::string_view>(), col, tab_width);
        } else {
            atom_width =
                utf8::char_width(line.substr(this->pos_.col_, utf8::len(line[this->pos_.col_])), col, tab_width);
        }

        if (col + atom_width > this->pref_col_) { break; }

        if (!this->step_forward(doc)) { break; }
        if (this->pos_.row_ != row) {
            this->step_backward(doc);
            break;
        }

        col += atom_width;
    }
}

void Cursor::down(const Document& doc, const std::size_t n) {
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }

    this->pos_.row_ = std::min(this->pos_.row_ + n, math::sub_sat(doc.line_count(), static_cast<std::size_t>(1)));
    this->pos_.col_ = 0;

    const auto row = this->pos_.row_;
    std::size_t col = 0;
    while (this->pos_.row_ == row) {
        const auto line = doc.line(this->pos_.row_);
        if (const auto len = line.ends_with('\n') ? line.size() - 1 : line.size(); this->pos_.col_ >= len) { break; }

        std::size_t atom_width = 0;
        const auto point = this->point(doc);

        if (const auto* const property = doc.get_raw_text_property(point, "replacement")) {
            atom_width = utf8::str_width(property->value_.as<std::string_view>(), col, tab_width);
        } else {
            atom_width =
                utf8::char_width(line.substr(this->pos_.col_, utf8::len(line[this->pos_.col_])), col, tab_width);
        }

        if (col + atom_width > this->pref_col_) { break; }

        if (!this->step_forward(doc)) { break; }
        if (this->pos_.row_ != row) {
            this->step_backward(doc);
            break;
        }

        col += atom_width;
    }
}

void Cursor::left(const Document& doc, const std::size_t n) {
    for (std::size_t i = 0; i < n && this->step_backward(doc); i += 1) {}

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::right(const Document& doc, const std::size_t n) {
    for (std::size_t i = 0; i < n && this->step_forward(doc); i += 1) {}

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

auto Cursor::current_char(const Document& doc) const -> std::size_t {
    if (this->pos_.row_ >= doc.line_count()) { return WEOF; }
    const auto line = doc.line(this->pos_.row_);
    if (this->pos_.col_ >= line.size()) { return '\n'; }
    return utf8::decode(line.substr(this->pos_.col_));
}

auto Cursor::step_forward(const Document& doc) -> bool {
    if (this->pos_.row_ >= doc.line_count()) { return false; }

    const auto point = this->point(doc);
    if (const auto* const property = doc.get_raw_text_property(point, "replacement"); property) {
        if (property->end_ >= doc.size()) { // Overflow.
            this->_jump_to_end_of_file(doc);
        } else {
            this->point(doc, property->end_);
        }

        return true;
    }

    const auto line = doc.line(this->pos_.row_);
    auto len = line.size();
    if (line.ends_with('\n')) { len -= 1; }

    if (this->pos_.col_ >= len) {
        if (this->pos_.row_ + 1 >= doc.line_count()) { return false; }
        this->pos_.row_ += 1;
        this->pos_.col_ = 0;
        return true;
    }

    this->pos_.col_ += utf8::len(line[this->pos_.col_]);

    return true;
}

auto Cursor::step_backward(const Document& doc) -> bool {
    auto moved = false;
    if (this->pos_.col_ > 0) {
        const auto line = doc.line(this->pos_.row_);

        // Move back one byte, then continue moving back until the start of the utf-8 byte sequence.
        // Continuation bytes start with 10xxxxxx.
        this->pos_.col_ -= 1;
        while (this->pos_.col_ > 0 && (static_cast<unsigned char>(line[this->pos_.col_]) & 0xC0) == 0x80) {
            this->pos_.col_ -= 1;
        }

        moved = true;
    } else {
        if (this->pos_.row_ > 0) {
            this->pos_.row_ -= 1;
            const auto line = doc.line(this->pos_.row_);
            this->pos_.col_ = line.size();
            if (line.ends_with('\n')) { this->pos_.col_ = math::sub_sat(this->pos_.col_, static_cast<std::size_t>(1)); }

            moved = true;
        }
    }

    if (!moved) { return false; }

    const auto point = this->point(doc);
    if (const auto* const property = doc.get_raw_text_property(point, "replacement"); property) {
        this->point(doc, property->start_);
    }

    return true;
}

// Disable ArrayBound warnings for this section as clang-tidy returns false positives for std::iswspace.
// In the internal implementation of std::iswspace it assumes isascii(c) can be true AND c > 255 which is false.
// NOLINTBEGIN(clang-analyzer-security.ArrayBound)

auto Cursor::peek_forward(const Document& doc) -> std::optional<std::size_t> {
    if (!this->step_forward(doc)) { return std::nullopt; }

    const auto ret = this->current_char(doc);
    this->step_backward(doc);
    return ret;
}

auto Cursor::peek_backward(const Document& doc) -> std::optional<std::size_t> {
    if (!this->step_backward(doc)) { return std::nullopt; }

    const auto ret = this->current_char(doc);
    this->step_forward(doc);
    return ret;
}

void Cursor::point(const Document& doc, std::size_t point) {
    point = std::min(point, doc.size());

    if (const auto* const property = doc.get_raw_text_property(point, "replacement")) { point = property->start_; }

    std::size_t row = 0;
    std::size_t col = 0;
    for (; row < doc.line_count(); row += 1) {
        const auto line = doc.line(row);
        if (point < line.size()) { break; }
        point -= line.size();
    }
    col = point;

    this->pos_ = {.row_ = row, .col_ = col};

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

auto Cursor::point(const Document& doc) const -> std::size_t {
    std::size_t pos = 0;
    for (std::size_t idx = 0; idx < this->pos_.row_; idx += 1) { pos += doc.line(idx).size(); }
    return pos + this->pos_.col_;
}

void Cursor::_jump_to_beginning_of_line(const Document& doc) {
    this->pos_.col_ = 0;

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_jump_to_end_of_line(const Document& doc) {
    const auto line = doc.line(this->pos_.row_);
    this->pos_.col_ = line.size();
    if (line.ends_with('\n')) { this->pos_.col_ = math::sub_sat(this->pos_.col_, static_cast<std::size_t>(1)); }

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_jump_to_beginning_of_file(const Document& doc) {
    this->point(doc, 0);

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_jump_to_end_of_file(const Document& doc) {
    this->pos_.row_ = math::sub_sat(doc.line_count(), static_cast<std::size_t>(1));
    this->_jump_to_end_of_line(doc);

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_next_word(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        const auto ch = this->current_char(doc);
        if (std::cmp_equal(ch, WEOF)) { goto EXIT; }

        if (std::iswalnum(static_cast<wint_t>(ch)) != 0) {
            while (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) != 0) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else {
            if (!this->step_forward(doc)) { goto EXIT; }
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        }
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_next_word_end(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        const auto ch = this->current_char(doc);
        if (std::cmp_equal(ch, WEOF)) { goto EXIT; }

        if (std::iswalnum(static_cast<wint_t>(ch)) != 0) {
            while (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) != 0) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
            if (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) != 0) {
                while (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) != 0) {
                    if (!this->step_forward(doc)) { goto EXIT; }
                }
            } else if (this->current_char(doc) != static_cast<std::size_t>(WEOF)) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else {
            if (!this->step_forward(doc)) { goto EXIT; }
        }
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_prev_word(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (!this->step_backward(doc)) { goto EXIT; }

        if (auto ch = this->current_char(doc); std::iswalnum(static_cast<wint_t>(ch))) {
            while (this->step_backward(doc)) {
                if (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) == 0) {
                    this->step_forward(doc);
                    break;
                }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (this->step_backward(doc)) {
                if (std::iswspace(static_cast<wint_t>(this->current_char(doc))) == 0) { break; }
            }
            ch = this->current_char(doc);
            if (std::iswalnum(static_cast<wint_t>(ch)) != 0) {
                while (this->step_backward(doc)) {
                    if (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) == 0) {
                        this->step_forward(doc);
                        break;
                    }
                }
            }
        } else {
            // Punctuation.
        }
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_prev_word_end(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (!this->step_backward(doc)) { goto EXIT; }

        if (const auto ch = this->current_char(doc); std::iswalnum(static_cast<wint_t>(ch))) {
            while (this->step_backward(doc)) {
                if (std::iswalnum(static_cast<wint_t>(this->current_char(doc))) == 0) { break; }
            }
            while (this->step_backward(doc)) {
                if (std::iswspace(static_cast<wint_t>(this->current_char(doc))) == 0) {
                    this->step_forward(doc);
                    break;
                }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (this->step_backward(doc)) {
                if (std::iswspace(static_cast<wint_t>(this->current_char(doc))) == 0) {
                    this->step_forward(doc);
                    break;
                }
            }
        } else {
            // Punctuation.
        }
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_next_whitespace(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
            if (!this->step_forward(doc)) { goto EXIT; }
        }
        while (this->current_char(doc) != 0 && std::iswspace(static_cast<wint_t>(this->current_char(doc))) == 0) {
            if (!this->step_forward(doc)) { goto EXIT; }
        }
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_prev_whitespace(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (!this->step_backward(doc)) { goto EXIT; }

        while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
            if (!this->step_backward(doc)) { goto EXIT; }
        }

        while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) == 0) {
            if (!this->step_backward(doc)) { goto EXIT; }
        }

        while (std::iswspace(static_cast<wint_t>(this->current_char(doc))) != 0) {
            if (!this->step_backward(doc)) { goto EXIT; }
        }

        this->step_forward(doc);
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_next_empty_line(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        bool found = false;
        for (std::size_t y = this->pos_.row_ + 1; y < doc.line_count(); y += 1) {
            if (const auto line = doc.line(y); line.empty() || line == "\n") {
                this->pos_.row_ = y;
                this->pos_.col_ = 0;
                found = true;
                break;
            }
        }
        if (!found) { this->_jump_to_end_of_file(doc); }
    }

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_prev_empty_line(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (this->pos_.row_ == 0) { goto EXIT; }

        bool found = false;
        for (std::size_t y = this->pos_.row_ - 1;; y -= 1) {
            if (const auto line = doc.line(y); line.empty() || line == "\n") {
                this->pos_.row_ = y;
                this->pos_.col_ = 0;
                found = true;
                break;
            }

            if (y == 0) { break; }
        }

        if (!found) { this->_jump_to_beginning_of_file(doc); }
    }

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::_jump_to_matching_opposite(const Document& doc) {
    const auto start = this->current_char(doc);

    std::size_t opening{};
    std::size_t closing{};
    bool forward{};

    switch (start) {
        case '(': {
            opening = '(';
            closing = ')';
            forward = true;
            break;
        }
        case '[': {
            opening = '[';
            closing = ']';
            forward = true;
            break;
        }
        case '{': {
            opening = '{';
            closing = '}';
            forward = true;
            break;
        }
        case '<': {
            opening = '<';
            closing = '>';
            forward = true;
            break;
        }
        case ')': {
            opening = ')';
            closing = '(';
            forward = false;
            break;
        }
        case ']': {
            opening = ']';
            closing = '[';
            forward = false;
            break;
        }
        case '}': {
            opening = '}';
            closing = '{';
            forward = false;
            break;
        }
        case '>': {
            opening = '>';
            closing = '<';
            forward = false;
            break;
        }
        default: return;
    }

    const Position start_pos = this->pos_;
    int depth = 1;

    if (forward) {
        while (this->step_forward(doc)) {
            if (const auto ch = this->current_char(doc); ch == opening) { // Opening bracket.
                depth += 1;
            } else if (ch == closing) { // Closing bracket.
                depth -= 1;
            }

            if (depth == 0) { goto EXIT; }
        }
    } else {
        while (this->step_backward(doc)) {
            if (const auto ch = this->current_char(doc); ch == opening) { // Closing bracket.
                depth += 1;
            } else if (ch == closing) { // Opening bracket.
                depth -= 1;
            }

            if (depth == 0) { goto EXIT; }
        }
    }

    // Restore if no matching opposite was found.
    this->pos_ = start_pos;

EXIT:
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = doc.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(doc, tab_width);
}

void Cursor::update_pref_col(const Document& doc, const std::size_t tab_width) {
    if (this->pos_.row_ < doc.line_count()) {
        this->pref_col_ = utf8::byte_to_idx(doc.line(this->pos_.row_), this->pos_.col_, tab_width);
    } else {
        this->pref_col_ = 0;
    }
}

// NOLINTEND(clang-analyzer-security.ArrayBound)
