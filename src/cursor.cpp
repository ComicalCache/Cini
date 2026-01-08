#include "cursor.hpp"

#include "document.hpp"
#include "util/math.hpp"
#include "util/utf8.hpp"

void Cursor::up(const Document& doc, const std::size_t n) {
    this->pos_.row_ = math::sub_sat(this->pos_.row_, n);

    const auto line = doc.line(this->pos_.row_);
    this->pos_.col_ = utf8::idx_to_byte(line, this->pref_col_, doc.tab_width_);

    if (line.ends_with('\n') && this->pos_.col_ == line.size()) { this->pos_.col_ -= 1; }
}

void Cursor::down(const Document& doc, const std::size_t n) {
    this->pos_.row_ = std::min(this->pos_.row_ + n, math::sub_sat(doc.line_count(), static_cast<std::size_t>(1)));

    const auto line = doc.line(this->pos_.row_);
    this->pos_.col_ = utf8::idx_to_byte(line, this->pref_col_, doc.tab_width_);

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
        } else if (this->pos_.row_ > 0) {
            this->pos_.row_ -= 1;
            const auto line = doc.line(this->pos_.row_);
            this->pos_.col_ = line.size();
            if (line.ends_with('\n')) {
                this->pos_.col_ = math::sub_sat(this->pos_.col_, static_cast<std::size_t>(1));
            }
        } else { break; }
    }

    this->update_pref_col(doc);
}

void Cursor::right(const Document& doc, const std::size_t n) {
    for (std::size_t i = 0; i < n; i += 1) {
        if (const auto line = doc.line(this->pos_.row_); this->pos_.col_ < line.size()) {
            if (line[this->pos_.col_] == '\n') {
                if (this->pos_.row_ + 1 < doc.line_count()) {
                    this->pos_.row_ += 1;
                    this->pos_.col_ = 0;
                } else { break; }
            } else {
                const auto len = utf8::len(line[this->pos_.col_]);
                if (this->pos_.col_ + len > line.size()) { break; }
                this->pos_.col_ += len;
            }
        } else { break; }
    }

    this->update_pref_col(doc);
}

void Cursor::move_to(const Document& doc, Position pos) {
    pos.row_ = std::min(pos.row_, math::sub_sat(doc.line_count(), static_cast<std::size_t>(1)));

    const auto line = doc.line(pos.row_);
    std::size_t max_col = line.size();
    if (line.ends_with('\n')) { max_col = math::sub_sat(max_col, static_cast<std::size_t>(1)); }
    pos.col_ = std::min(pos.col_, max_col);

    this->pos_ = pos;
    this->update_pref_col(doc);
}

void Cursor::jump_to_beginning_of_line(const Document& doc) {
    this->pos_.col_ = 0;
    this->update_pref_col(doc);
}

void Cursor::jump_to_end_of_line(const Document& doc) {
    const auto line = doc.line(this->pos_.row_);
    this->pos_.col_ = line.size();
    if (line.ends_with('\n')) { this->pos_.col_ = math::sub_sat(this->pos_.col_, static_cast<std::size_t>(1)); }

    this->update_pref_col(doc);
}

void Cursor::jump_to_beginning_of_file(const Document& doc) {
    this->move_to(doc, {0, 0});

    this->update_pref_col(doc);
}

void Cursor::jump_to_end_of_file(const Document& doc) {
    this->pos_.row_ = math::sub_sat(doc.line_count(), static_cast<std::size_t>(1));
    this->jump_to_end_of_line(doc);

    this->update_pref_col(doc);
}

void Cursor::next_word(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        const auto ch = this->current_char(doc);
        if (ch == static_cast<std::size_t>(WEOF)) { goto EXIT; }

        if (std::iswalnum(static_cast<wint_t>(ch))) {
            while (std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch))) {
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else {
            if (!this->step_forward(doc)) { goto EXIT; }
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        }
    }

EXIT:
    this->update_pref_col(doc);
}

void Cursor::next_word_end(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        const auto ch = this->current_char(doc);
        if (ch == static_cast<std::size_t>(WEOF)) { goto EXIT; }

        if (std::iswalnum(static_cast<wint_t>(ch))) {
            while (std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch))) {
            while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
            if (std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) {
                while (std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) {
                    if (!this->step_forward(doc)) { goto EXIT; }
                }
            } else if (this->current_char(doc) != static_cast<std::size_t>(WEOF)) {
                if (!this->step_forward(doc)) { goto EXIT; }
            }
        } else { if (!this->step_forward(doc)) { goto EXIT; } }
    }

EXIT:
    this->update_pref_col(doc);
}

void Cursor::prev_word(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (!this->step_backward(doc)) { goto EXIT; }

        if (auto ch = this->current_char(doc); std::iswalnum(static_cast<wint_t>(ch))) {
            while (this->step_backward(doc)) {
                if (!std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) {
                    this->step_forward(doc);
                    break;
                }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch))) {
            while (this->step_backward(doc)) {
                if (!std::iswspace(static_cast<wint_t>(this->current_char(doc)))) { break; }
            }
            ch = this->current_char(doc);
            if (std::iswalnum(static_cast<wint_t>(ch))) {
                while (this->step_backward(doc)) {
                    if (!std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) {
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
    this->update_pref_col(doc);
}

void Cursor::prev_word_end(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (!this->step_backward(doc)) { goto EXIT; }

        if (const auto ch = this->current_char(doc); std::iswalnum(static_cast<wint_t>(ch))) {
            while (this->step_backward(doc)) {
                if (!std::iswalnum(static_cast<wint_t>(this->current_char(doc)))) { break; }
            }
            while (this->step_backward(doc)) {
                if (!std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
                    this->step_forward(doc);
                    break;
                }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch))) {
            while (this->step_backward(doc)) {
                if (!std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
                    this->step_forward(doc);
                    break;
                }
            }
        } else {
            // Punctuation.
        }
    }

EXIT:
    this->update_pref_col(doc);
}

void Cursor::next_whitespace(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
            if (!this->step_forward(doc)) { goto EXIT; }
        }
        while (this->current_char(doc) != 0 && !std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
            if (!this->step_forward(doc)) { goto EXIT; }
        }
    }

EXIT:
    this->update_pref_col(doc);
}

void Cursor::prev_whitespace(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (!this->step_backward(doc)) { goto EXIT; }

        while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
            if (!this->step_backward(doc)) { goto EXIT; }
        }

        while (!std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
            if (!this->step_backward(doc)) { goto EXIT; }
        }

        while (std::iswspace(static_cast<wint_t>(this->current_char(doc)))) {
            if (!this->step_backward(doc)) { goto EXIT; }
        }

        this->step_forward(doc);
    }

EXIT:
    this->update_pref_col(doc);
}

void Cursor::next_empty_line(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        bool found = false;
        for (std::size_t y = this->pos_.row_ + 1; y < doc.line_count(); ++y) {
            if (const auto line = doc.line(y); line.empty() || line == "\n") {
                this->pos_.row_ = y;
                this->pos_.col_ = 0;
                found = true;
                break;
            }
        }
        if (!found) { this->jump_to_end_of_file(doc); }
    }

    this->update_pref_col(doc);
}

void Cursor::prev_empty_line(const Document& doc, const std::size_t n) {
    for (std::size_t idx = 0; idx < n; idx += 1) {
        if (this->pos_.row_ == 0) { goto EXIT; }

        bool found = false;
        for (std::size_t y = this->pos_.row_ - 1; ; --y) {
            if (const auto line = doc.line(y); line.empty() || line == "\n") {
                this->pos_.row_ = y;
                this->pos_.col_ = 0;
                found = true;
                break;
            }

            if (y == 0) { break; }
        }

        if (!found) { this->jump_to_beginning_of_file(doc); }
    }

EXIT:
    this->update_pref_col(doc);
}

void Cursor::jump_to_matching_opposite(const Document& doc) {
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
    this->update_pref_col(doc);
}

std::size_t Cursor::byte(const Document& doc) const {
    std::size_t pos = 0;
    for (std::size_t idx = 0; idx < this->pos_.row_; idx += 1) { pos += doc.line(idx).size(); }
    return pos + this->pos_.col_;
}

void Cursor::update_pref_col(const Document& doc) {
    if (this->pos_.row_ < doc.line_count()) {
        this->pref_col_ = utf8::byte_to_idx(doc.line(this->pos_.row_), this->pos_.col_, doc.tab_width_);
    } else { this->pref_col_ = 0; }
}

std::size_t Cursor::current_char(const Document& doc) const {
    if (this->pos_.row_ >= doc.line_count()) { return WEOF; }
    const auto line = doc.line(this->pos_.row_);
    if (this->pos_.col_ >= line.size()) { return '\n'; }
    return utf8::decode(line.substr(this->pos_.col_));
}

bool Cursor::step_forward(const Document& doc) {
    if (this->pos_.row_ >= doc.line_count()) { return false; }

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

bool Cursor::step_backward(const Document& doc) {
    if (this->pos_.col_ > 0) {
        const auto line = doc.line(this->pos_.row_);

        // Move back one byte, then continue moving back until the start of the utf-8 byte sequence.
        // Continuation bytes start with 10xxxxxx.
        this->pos_.col_ -= 1;
        while (this->pos_.col_ > 0 && (static_cast<unsigned char>(line[this->pos_.col_]) & 0xC0) == 0x80) {
            this->pos_.col_ -= 1;
        }

        return true;
    } else {
        if (this->pos_.row_ == 0) { return false; }

        this->pos_.row_ -= 1;
        const auto line = doc.line(this->pos_.row_);
        this->pos_.col_ = line.size();
        if (line.ends_with('\n')) {
            this->pos_.col_ = math::sub_sat(this->pos_.col_, static_cast<std::size_t>(1));
        }

        return true;
    }
}
