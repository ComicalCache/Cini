#include "display.hpp"

#include "../types/face.hpp"
#include "../util/assert.hpp"

Display::Display() {
    // Store instance in the write request to have access to this in callback.
    this->write_req_.data = this;
    this->output_buffer_.reserve(4096);
}

void Display::resize(const std::size_t width, const std::size_t height) {
    if (this->width_ == width && this->height_ == height) { return; }

    this->width_ = width;
    this->height_ = height;

    this->grid_.resize(height);
    for (auto& row: this->grid_) { row.resize(width, Cell(" ", Face{})); }

    this->full_redraw_ = true;
}

void Display::update(std::size_t x, std::size_t y, const Cell& cell) {
    ASSERT_DEBUG // NOLINT(readability-simplify-boolean-expr)
        (x < this->width_ && y < this->height_, "Coordinates must be inside screen space.");

    // Always overwrite on full redraw since the old state is invalidated.
    if (this->full_redraw_ || this->grid_[y][x] != cell) {
        this->grid_[y][x] = cell;

        // Only push dirty if not a full redraw since all cells get drawn on full redraw anyway.
        if (!this->full_redraw_) { this->dirty_.emplace_back(x, y); }
    }
}

void Display::cursor(const std::size_t row, const std::size_t col, const ansi::CursorStyle style) {
    ASSERT_DEBUG // NOLINT(readability-simplify-boolean-expr)
        (col < this->width_ && row < this->height_, "Cursor must be inside screen space.");

    this->cur_.row_ = row + 1;
    this->cur_.col_ = col + 1;
    this->cur_style_ = style;
}

void Display::render(uv_tty_t* tty) {
    // The previous render pass has not been finished by libuv yet, abort.
    if (this->is_writing_) { return; }

    this->back_buffer_.clear();

    // Avoid flickering during writing.
    ansi::hide_cursor(this->back_buffer_);

    if (this->full_redraw_) {
        ansi::clear(this->back_buffer_);

        Display::StyleCache last_style{};
        for (auto y{0UZ}; y < this->height_; y += 1) {
            for (auto x{0UZ}; x < this->width_; x += 1) { this->render_cell(x, y, this->grid_[y][x], last_style); }
        }

        this->full_redraw_ = false;
    } else if (!this->dirty_.empty()) {
        Display::StyleCache last_style{};
        for (const auto& [x, y]: this->dirty_) { this->render_cell(x, y, this->grid_[y][x], last_style); }
    }
    this->dirty_.clear();

    // Reset style after rendering to avoid side effects on the terminal.
    ansi::reset_style(this->back_buffer_);

    ansi::move_to(this->back_buffer_, this->cur_.row_, this->cur_.col_);
    ansi::cursor(this->back_buffer_, this->cur_style_);
    if (this->cur_style_ != ansi::CursorStyle::HIDDEN) { ansi::show_cursor(this->back_buffer_); }

    this->flush(tty);
}

void Display::render_cell(const std::size_t x, const std::size_t y, const Cell& cell, Display::StyleCache& last_style) {
    // Cells with length 0 won't be rendered, since nothing would be seen.
    if (cell.len_ == 0) { return; }

    ansi::move_to(this->back_buffer_, y + 1, x + 1);

    // Only write and update color if it changed.
    if (!last_style.fg_.has_value() || *last_style.fg_ != cell.fg_) {
        ansi::rgb(this->back_buffer_, cell.fg_);
        last_style.fg_ = cell.fg_;
    }
    if (!last_style.bg_.has_value() || *last_style.bg_ != cell.bg_) {
        ansi::rgb(this->back_buffer_, cell.bg_, false);
        last_style.bg_ = cell.bg_;
    }
    if (!last_style.uc_.has_value() || *last_style.uc_ != cell.uc_) {
        ansi::underline_rgb(this->back_buffer_, cell.uc_);
        last_style.uc_ = cell.uc_;
    }

    // Only write and update style if it changed.
    if (!last_style.bold_.has_value() || *last_style.bold_ != cell.bold_) {
        ansi::bold(this->back_buffer_, cell.bold_);
        last_style.bold_ = cell.bold_;
    }
    if (!last_style.italic_.has_value() || *last_style.italic_ != cell.italic_) {
        ansi::italic(this->back_buffer_, cell.italic_);
        last_style.italic_ = cell.italic_;
    }
    if (!last_style.underline_.has_value() || *last_style.underline_ != cell.underline_) {
        ansi::underline(this->back_buffer_, cell.underline_);
        last_style.underline_ = cell.underline_;
    }
    if (!last_style.squiggly_.has_value() || *last_style.squiggly_ != cell.squiggly_) {
        ansi::squiggly(this->back_buffer_, cell.squiggly_);
        last_style.squiggly_ = cell.squiggly_;
    }
    if (!last_style.strikethrough_.has_value() || *last_style.strikethrough_ != cell.strikethrough_) {
        ansi::strikethrough(this->back_buffer_, cell.strikethrough_);
        last_style.strikethrough_ = cell.strikethrough_;
    }

    // Add the cell's Unicode codepoint.
    this->back_buffer_.append(cell.data_.data(), cell.data_.data() + cell.len_);
}

void Display::flush(uv_tty_t* tty) {
    if (this->back_buffer_.empty()) { return; }

    // Double buffering.
    this->output_buffer_ = std::move(this->back_buffer_);

    this->back_buffer_.clear();
    this->back_buffer_.reserve(output_buffer_.size());

    // Write to stdout via libuv.
    this->is_writing_ = true;
    const auto buf{uv_buf_init(this->output_buffer_.data(), this->output_buffer_.size())};
    uv_write(&this->write_req_, reinterpret_cast<uv_stream_t*>(tty), &buf, 1, [](uv_write_t* req, int) -> void {
        auto* self{static_cast<Display*>(req->data)};
        self->is_writing_ = false;

        // Call callback when redraw was requested while drawing.
        if ((self->full_redraw_ || !self->dirty_.empty()) && self->ready_) { self->ready_(); }
    });
}
