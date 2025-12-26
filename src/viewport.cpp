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

    for (std::size_t y = 0; y < this->height_; ++y) {
        const auto doc_y = this->scroll_.row_ + y;

        if (this->gutter_) {
            if (doc_y < this->doc_->line_count()) {
                auto line_num = std::format("{:>{}} ", doc_y + 1, gutter_width - 1);
                for (std::size_t x = 0; x < gutter_width; ++x) {
                    Cell c;
                    c.set_char(line_num[x]);
                    c.fg_ = {100, 100, 100};
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
                }
            } else { // Clear the line as it contains no data.
                for (std::size_t x = 0; x < this->width_; ++x) {
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, Cell(" "));
                }
            }
        }

        if (doc_y < this->doc_->line_count()) {
            auto line = this->doc_->line(doc_y);

            std::size_t x = 0;
            std::size_t idx = 0;
            while (idx < line.length()) {
                const auto len = util::utf8_len(line[idx]);
                const auto ch = line.substr(idx, len);

                // Layer 1: character replacement.
                Cell cell;
                if (auto it = this->replacements_.find(ch); it != this->replacements_.end()) {
                    cell = it->second;
                } else { cell.set_utf8(ch); }

                const auto width = util::char_width(ch, x);
                if (x + width > this->scroll_.col_ && x < this->scroll_.col_ + (this->width_ - gutter_width)) {
                    for (std::size_t n = 0; n < width; ++n) {
                        auto vx = x + n;

                        if (vx < this->scroll_.col_) continue;
                        if (vx >= this->scroll_.col_ + (this->width_ - gutter_width)) break;

                        vx = vx - this->scroll_.col_;

                        if (n == 0) { // Draw character.
                            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + y, cell);
                        } else { // Expand tab or wide characters.
                            Cell filler;
                            if (ch == "\t") { filler = Cell(" "); }
                            filler.bg_ = cell.bg_;

                            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + y, filler);
                        }
                    }
                }

                x += width;
                idx += len;
            }

            // Fill remainder of line.
            for (; x < this->scroll_.col_ + (this->width_ - gutter_width); ++x) {
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
    while (idx < this->cur_.pos_.col_ && idx < line.length()) {
        const auto len = util::utf8_len(line[idx]);
        const auto ch = line.substr(idx, len);

        x += util::char_width(ch, x);
        idx += len;
    }

    // Don't draw cursors outside the viewport.
    if (x < this->scroll_.col_ || x >= this->scroll_.col_ + (this->width_ - gutter)) { return; }
    x = x - this->scroll_.col_;

    display.cursor(this->offset_.row_ + y, this->offset_.col_ + gutter + x);
}
