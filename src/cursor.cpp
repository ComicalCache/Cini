#include "cursor.hpp"

#include "document.hpp"
#include "document_view.hpp"
#include "util/assert.hpp"
#include "util/math.hpp"
#include "util/utf8.hpp"

void Cursor::up(const DocumentView& view, const std::size_t n) {
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }

    this->pos_.row_ = math::sub_sat(this->pos_.row_, n);
    this->pos_.col_ = 0;

    // FIXME: dedupe this with Cursor::down.
    const auto row = this->pos_.row_;
    auto col{0UZ};
    while (this->pos_.row_ == row) {
        const auto line = view.doc_->line(this->pos_.row_);
        if (const auto len = line.ends_with('\n') ? line.size() - 1 : line.size(); this->pos_.col_ >= len) { break; }

        auto atom_width{0UZ};
        const auto point = this->point(view);

        if (const auto* const property = view.get_raw_view_property(point, "replacement")) {
            atom_width = utf8::str_width(property->value_.as<std::string_view>(), col, tab_width);
        } else {
            atom_width =
                utf8::char_width(line.substr(this->pos_.col_, utf8::len(line[this->pos_.col_])), col, tab_width);
        }

        if (col + atom_width > this->pref_col_) { break; }

        if (!this->step_forward(view)) { break; }
        if (this->pos_.row_ != row) {
            this->step_backward(view);
            break;
        }

        col += atom_width;
    }
}

void Cursor::down(const DocumentView& view, const std::size_t n) {
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }

    this->pos_.row_ = std::min(this->pos_.row_ + n, math::sub_sat(view.doc_->line_count(), 1UZ));
    this->pos_.col_ = 0;

    // FIXME: dedupe this with Cursor::up.
    const auto row = this->pos_.row_;
    auto col{0UZ};
    while (this->pos_.row_ == row) {
        const auto line = view.doc_->line(this->pos_.row_);
        if (const auto len = line.ends_with('\n') ? line.size() - 1 : line.size(); this->pos_.col_ >= len) { break; }

        auto atom_width{0UZ};
        const auto point = this->point(view);

        if (const auto* const property = view.get_raw_view_property(point, "replacement")) {
            atom_width = utf8::str_width(property->value_.as<std::string_view>(), col, tab_width);
        } else {
            atom_width =
                utf8::char_width(line.substr(this->pos_.col_, utf8::len(line[this->pos_.col_])), col, tab_width);
        }

        if (col + atom_width > this->pref_col_) { break; }

        if (!this->step_forward(view)) { break; }
        if (this->pos_.row_ != row) {
            this->step_backward(view);
            break;
        }

        col += atom_width;
    }
}

void Cursor::left(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n && this->step_backward(view); idx += 1) {}

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::right(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n && this->step_forward(view); idx += 1) {}

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

auto Cursor::current_char(const DocumentView& view) const -> std::size_t {
    if (this->pos_.row_ >= view.doc_->line_count()) { return WEOF; }

    const auto line = view.doc_->line(this->pos_.row_);
    if (this->pos_.col_ >= line.size()) { return '\n'; }

    return utf8::decode(line.substr(this->pos_.col_));
}

auto Cursor::step_forward(const DocumentView& view) -> bool {
    if (this->pos_.row_ >= view.doc_->line_count()) { return false; }

    const auto point = this->point(view);
    if (const auto* const property = view.get_raw_view_property(point, "replacement"); property) {
        if (property->end_ >= view.doc_->size()) { // Overflows the Document.
            this->_jump_to_end_of_file(view);
        } else {
            this->point(view, property->end_);
        }

        return true;
    }

    const auto line = view.doc_->line(this->pos_.row_);
    auto len = line.size();

    // Do not treat the newline character as a "character".
    if (line.ends_with('\n')) { len -= 1; }

    // End of line.
    if (this->pos_.col_ >= len) {
        // End of Document.
        if (this->pos_.row_ + 1 >= view.doc_->line_count()) { return false; }

        this->pos_.row_ += 1;
        this->pos_.col_ = 0;
        return true;
    }

    this->pos_.col_ += utf8::len(line[this->pos_.col_]);

    return true;
}

auto Cursor::step_backward(const DocumentView& view) -> bool {
    auto moved{false};
    if (this->pos_.col_ > 0) {
        const auto line = view.doc_->line(this->pos_.row_);

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
            const auto line = view.doc_->line(this->pos_.row_);
            this->pos_.col_ = line.size();

            // Do not treat the newline character as a "character".
            if (line.ends_with('\n')) { this->pos_.col_ = math::sub_sat(this->pos_.col_, 1UZ); }

            moved = true;
        }
    }

    if (!moved) { return false; }

    const auto point = this->point(view);
    if (const auto* const property = view.get_raw_view_property(point, "replacement"); property) {
        this->point(view, property->start_);
    }

    return true;
}

// Disable ArrayBound warnings for this section as clang-tidy returns false positives for std::iswspace.
// In the internal implementation of std::iswspace it assumes isascii(c) can be true AND c > 255, which is false.
// NOLINTBEGIN(clang-analyzer-security.ArrayBound)

auto Cursor::peek_forward(const DocumentView& view) -> std::optional<std::size_t> {
    if (!this->step_forward(view)) { return std::nullopt; }

    const auto ret = this->current_char(view);
    this->step_backward(view);
    return ret;
}

auto Cursor::peek_backward(const DocumentView& view) -> std::optional<std::size_t> {
    if (!this->step_backward(view)) { return std::nullopt; }

    const auto ret = this->current_char(view);
    this->step_forward(view);
    return ret;
}

void Cursor::point(const DocumentView& view, std::size_t point) {
    ASSERT(point <= view.doc_->size(), "");

    if (const auto* const property = view.get_raw_view_property(point, "replacement")) {
        point = std::min(property->start_, point);
    }

    this->pos_ = view.doc_->position_from_byte(point);

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

auto Cursor::point(const DocumentView& view) const -> std::size_t {
    return view.doc_->line_begin_byte(this->pos_.row_) + this->pos_.col_;
}

void Cursor::_jump_to_beginning_of_line(const DocumentView& view) {
    this->pos_.col_ = 0;

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_jump_to_end_of_line(const DocumentView& view) {
    const auto line = view.doc_->line(this->pos_.row_);
    this->pos_.col_ = line.size();

    // Do not treat the newline character as a "character".
    if (line.ends_with('\n')) { this->pos_.col_ = math::sub_sat(this->pos_.col_, 1UZ); }

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_jump_to_beginning_of_file(const DocumentView& view) {
    this->point(view, 0);

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_jump_to_end_of_file(const DocumentView& view) {
    this->pos_.row_ = math::sub_sat(view.doc_->line_count(), 1UZ);
    this->_jump_to_end_of_line(view);

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_next_word(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        const auto ch = this->current_char(view);
        if (std::cmp_equal(ch, WEOF)) { goto EXIT; }

        if (std::iswalnum(static_cast<wint_t>(ch)) != 0) {
            while (std::iswalnum(static_cast<wint_t>(this->current_char(view))) != 0) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
            while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
        } else {
            if (!this->step_forward(view)) { goto EXIT; }
            while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
        }
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_next_word_end(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        const auto ch = this->current_char(view);
        if (std::cmp_equal(ch, WEOF)) { goto EXIT; }

        if (std::iswalnum(static_cast<wint_t>(ch)) != 0) {
            while (std::iswalnum(static_cast<wint_t>(this->current_char(view))) != 0) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
            if (std::iswalnum(static_cast<wint_t>(this->current_char(view))) != 0) {
                while (std::iswalnum(static_cast<wint_t>(this->current_char(view))) != 0) {
                    if (!this->step_forward(view)) { goto EXIT; }
                }
            } else if (this->current_char(view) != static_cast<std::size_t>(WEOF)) {
                if (!this->step_forward(view)) { goto EXIT; }
            }
        } else {
            if (!this->step_forward(view)) { goto EXIT; }
        }
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_prev_word(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        if (!this->step_backward(view)) { goto EXIT; }

        if (auto ch = this->current_char(view); std::iswalnum(static_cast<wint_t>(ch))) {
            while (this->step_backward(view)) {
                if (std::iswalnum(static_cast<wint_t>(this->current_char(view))) == 0) {
                    this->step_forward(view);
                    break;
                }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (this->step_backward(view)) {
                if (std::iswspace(static_cast<wint_t>(this->current_char(view))) == 0) { break; }
            }
            ch = this->current_char(view);
            if (std::iswalnum(static_cast<wint_t>(ch)) != 0) {
                while (this->step_backward(view)) {
                    if (std::iswalnum(static_cast<wint_t>(this->current_char(view))) == 0) {
                        this->step_forward(view);
                        break;
                    }
                }
            }
        } else {
            // Punctuation.
        }
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_prev_word_end(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        if (!this->step_backward(view)) { goto EXIT; }

        if (const auto ch = this->current_char(view); std::iswalnum(static_cast<wint_t>(ch))) {
            while (this->step_backward(view)) {
                if (std::iswalnum(static_cast<wint_t>(this->current_char(view))) == 0) { break; }
            }
            while (this->step_backward(view)) {
                if (std::iswspace(static_cast<wint_t>(this->current_char(view))) == 0) {
                    this->step_forward(view);
                    break;
                }
            }
        } else if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            while (this->step_backward(view)) {
                if (std::iswspace(static_cast<wint_t>(this->current_char(view))) == 0) {
                    this->step_forward(view);
                    break;
                }
            }
        } else {
            // Punctuation.
        }
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_next_whitespace(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
            if (!this->step_forward(view)) { goto EXIT; }
        }
        while (this->current_char(view) != 0 && std::iswspace(static_cast<wint_t>(this->current_char(view))) == 0) {
            if (!this->step_forward(view)) { goto EXIT; }
        }
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_prev_whitespace(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        if (!this->step_backward(view)) { goto EXIT; }

        while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
            if (!this->step_backward(view)) { goto EXIT; }
        }

        while (std::iswspace(static_cast<wint_t>(this->current_char(view))) == 0) {
            if (!this->step_backward(view)) { goto EXIT; }
        }

        while (std::iswspace(static_cast<wint_t>(this->current_char(view))) != 0) {
            if (!this->step_backward(view)) { goto EXIT; }
        }

        this->step_forward(view);
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_next_empty_line(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        auto found{false};
        for (auto y = this->pos_.row_ + 1; y < view.doc_->line_count(); y += 1) {
            if (const auto line = view.doc_->line(y); line.empty() || line == "\n") {
                this->pos_.row_ = y;
                this->pos_.col_ = 0;
                found = true;
                break;
            }
        }

        if (!found) { this->_jump_to_end_of_file(view); }
    }

    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_prev_empty_line(const DocumentView& view, const std::size_t n) {
    for (auto idx{0UZ}; idx < n; idx += 1) {
        if (this->pos_.row_ == 0) { goto EXIT; }

        auto found{false};
        for (auto y = this->pos_.row_ - 1;; y -= 1) {
            if (const auto line = view.doc_->line(y); line.empty() || line == "\n") {
                this->pos_.row_ = y;
                this->pos_.col_ = 0;
                found = true;
                break;
            }

            if (y == 0) { break; }
        }

        if (!found) { this->_jump_to_beginning_of_file(view); }
    }

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::_jump_to_matching_opposite(const DocumentView& view) {
    const auto start = this->current_char(view);

    std::size_t opening{0UZ};
    std::size_t closing{0UZ};
    auto forward{false};

    switch (start) {
        case '(':
            opening = '(';
            closing = ')';
            forward = true;
            break;
        case '[':
            opening = '[';
            closing = ']';
            forward = true;
            break;
        case '{':
            opening = '{';
            closing = '}';
            forward = true;
            break;
        case '<':
            opening = '<';
            closing = '>';
            forward = true;
            break;
        case ')':
            opening = ')';
            closing = '(';
            forward = false;
            break;
        case ']':
            opening = ']';
            closing = '[';
            forward = false;
            break;
        case '}':
            opening = '}';
            closing = '{';
            forward = false;
            break;
        case '>':
            opening = '>';
            closing = '<';
            forward = false;
            break;
        default: return;
    }

    const Position start_pos = this->pos_;
    auto depth{1UZ};

    if (forward) {
        while (this->step_forward(view)) {
            if (const auto ch = this->current_char(view); ch == opening) { // Opening bracket.
                depth += 1;
            } else if (ch == closing) { // Closing bracket.
                depth -= 1;
            }

            if (depth == 0) { goto EXIT; }
        }
    } else {
        while (this->step_backward(view)) {
            if (const auto ch = this->current_char(view); ch == opening) { // Closing bracket.
                depth += 1;
            } else if (ch == closing) { // Opening bracket.
                depth -= 1;
            }

            if (depth == 0) { goto EXIT; }
        }
    }

    // Restore if no matching opposite was found.
    this->pos_ = start_pos;
    return;

EXIT:
    auto tab_width{4UZ};
    if (const sol::optional<std::size_t> t = view.properties_["tab_width"]; t) { tab_width = *t; }
    this->update_pref_col(view, tab_width);
}

void Cursor::update_pref_col(const DocumentView& view, const std::size_t tab_width) {
    if (this->pos_.row_ < view.doc_->line_count()) {
        this->pref_col_ = utf8::byte_to_idx(view.doc_->line(this->pos_.row_), this->pos_.col_, tab_width);
    } else {
        this->pref_col_ = 0;
    }
}

// NOLINTEND(clang-analyzer-security.ArrayBound)
