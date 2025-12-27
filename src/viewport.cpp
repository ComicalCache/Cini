#include "viewport.hpp"

#include <cassert>
#include <cmath>
#include <format>
#include <utility>

#include "cell.hpp"
#include "util.hpp"

Viewport::Viewport(const std::size_t width, const std::size_t height, std::shared_ptr<Document> doc)
    : doc_{std::move(doc)}, replacements_{{"\t", Cell("↦")}, {" ", Cell("·")}, {"\r", Cell("↤")}, {"\n", Cell("⏎")}},
      width_{width}, height_{height} { assert(this->doc_ != nullptr); }

void Viewport::move_cursor(const cursor::move_fn move_fn, const std::size_t n) {
    (this->cur_.*move_fn)(*this->doc_, n);
    this->adjust_viewport();
}

void Viewport::scroll_up(const std::size_t n) { this->scroll_.row_ = util::math::sub_sat(this->scroll_.row_, n); }

void Viewport::scroll_down(const std::size_t n) {
    if (const auto max_scroll = this->doc_->line_count(); this->scroll_.row_ + n < max_scroll) {
        this->scroll_.row_ += n;
    } else { this->scroll_.row_ = max_scroll; }
}

void Viewport::scroll_left(const std::size_t n) { this->scroll_.col_ = util::math::sub_sat(this->scroll_.col_, n); }

void Viewport::scroll_right(const std::size_t n) { this->scroll_.col_ += n; }

void Viewport::resize(const std::size_t width, const std::size_t height, const Position offset) {
    if (this->width_ == width && this->height_ == height) { return; }

    this->width_ = width;
    this->height_ = height;
    this->offset_ = offset;
}

void Viewport::render(Display& display) const {
    assert(this->doc_ != nullptr);

    std::size_t gutter_width = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter_width = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    for (std::size_t y = 0; y < this->height_; y += 1) {
        const auto doc_y = this->scroll_.row_ + y;

        if (this->gutter_) {
            if (doc_y < this->doc_->line_count()) {
                auto line_num = std::format("{:>{}} ", doc_y + 1, gutter_width - 1);
                for (std::size_t x = 0; x < gutter_width; x += 1) {
                    Cell c;
                    c.set_char(line_num[x]);
                    c.fg_ = {100, 100, 100};
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
                }
            } else { // Clear the line as it contains no data.
                for (std::size_t x = 0; x < this->width_; x += 1) {
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, Cell(" "));
                }
            }
        }

        if (doc_y < this->doc_->line_count()) {
            auto line = this->doc_->line(doc_y);

            std::size_t x = 0;
            std::size_t idx = 0;
            while (idx < line.length()) {
                const auto len = util::utf8::len(line[idx]);
                const auto ch = line.substr(idx, len);

                // Character replacement.
                Cell cell;
                if (auto it = this->replacements_.find(ch); it != this->replacements_.end()) {
                    cell = it->second;
                } else { cell.set_utf8(ch); }

                const auto width = util::char_width(ch, x);
                if (x + width > this->scroll_.col_ && x < this->scroll_.col_ + (this->width_ - gutter_width)) {
                    for (std::size_t n = 0; n < width; n += 1) {
                        auto vx = x + n;

                        if (vx < this->scroll_.col_) continue;
                        if (vx >= this->scroll_.col_ + (this->width_ - gutter_width)) break;

                        vx = vx - this->scroll_.col_;

                        if (n == 0) { // Draw character.
                            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + y, cell);
                        } else { // Expand tab or wide characters.
                            Cell filler;
                            if (ch == "\t") { // Tab.
                                filler = Cell(" ");
                            } else if (x < this->scroll_.col_) { // Half cutoff wide character.
                                filler = Cell("▯");
                            }
                            filler.bg_ = cell.bg_;

                            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + y, filler);
                        }
                    }
                }

                x += width;
                idx += len;
            }

            // Fill remainder of line.
            for (; x < this->scroll_.col_ + (this->width_ - gutter_width); x += 1) {
                if (x >= this->scroll_.col_) {
                    const std::size_t screen_x = x - this->scroll_.col_ + gutter_width;
                    display.update(this->offset_.col_ + screen_x, this->offset_.row_ + y, Cell(" "));
                }
            }
        }
    }
}

void Viewport::render_cursor(Display& display) const {
    // Don't draw cursors outside the viewport.
    if (this->cur_.pos_.row_ < this->scroll_.row_ || this->cur_.pos_.row_ >= this->scroll_.row_ + this->height_) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }
    const auto y = this->cur_.pos_.row_ - this->scroll_.row_;

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    const auto line = this->doc_->line(y);

    std::size_t x = 0;
    std::size_t idx = 0;
    while (idx < line.length()) {
        const auto len = util::utf8::len(line[idx]);
        const auto width = util::char_width(line.substr(idx, len), x);

        if (x + width > this->cur_.pref_col_) { break; }

        x += width;
        idx += len;
    }

    // Don't draw cursors outside the viewport.
    if (x < this->scroll_.col_ || x >= this->scroll_.col_ + (this->width_ - gutter)) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }
    x = x - this->scroll_.col_;

    display.cursor(this->offset_.row_ + y, this->offset_.col_ + gutter + x);
}

void Viewport::adjust_viewport() {
    if (this->doc_->line_count() == 0) return;

    // 1. Vertical scrolling.
    if (this->cur_.pos_.row_ < this->scroll_.row_) { // Above.
        this->scroll_.row_ = this->cur_.pos_.row_;
    } else if (this->cur_.pos_.row_ >= this->scroll_.row_ + this->height_) { // Bellow.
        this->scroll_.row_ = this->cur_.pos_.row_ - this->height_ + 1;
    }

    if (this->cur_.pos_.row_ >= this->doc_->line_count()) return;

    const auto line = this->doc_->line(this->cur_.pos_.row_);

    // 2. Horizontal scrolling.
    std::size_t x = 0;
    std::size_t idx = 0;
    while (idx < line.length()) {
        const auto len = util::utf8::len(line[idx]);
        const auto width = util::char_width(line.substr(idx, len), x);

        if (x + width > this->cur_.pref_col_) { break; }

        x += width;
        idx += len;
    }

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    if (x < this->scroll_.col_) { // Left.
        this->scroll_.col_ = x;
    } else if (x >= this->scroll_.col_ + this->width_ - gutter) { // Right.
        this->scroll_.col_ = x - this->width_ - gutter + 1;
    }
}
