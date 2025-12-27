#include "display.hpp"

#include <cassert>

#include "ansi.hpp"

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
    for (auto& row: this->grid_) { row.resize(width, Cell(" ")); }

    this->full_redraw_ = true;
}

void Display::update(std::size_t x, std::size_t y, const Cell& cell) {
    assert(x < this->width_ && y < this->height_);

    // Always overwrite on full redraw since the old state is invalidated.
    if (this->full_redraw_ || this->grid_[y][x] != cell) {
        this->grid_[y][x] = cell;

        // Only push dirty if not a full redraw since all cells get drawn on full redraw anyway.
        if (!this->full_redraw_) { this->dirty_.emplace_back(x, y); }
    }
}

void Display::cursor(const std::size_t row, const std::size_t col, const ansi::CursorStyle style) {
    assert(col < this->width_ && row < this->height_);

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

        std::optional<Rgb> last_fg{}, last_bg{};
        for (std::size_t y = 0; y < this->height_; y += 1) {
            for (std::size_t x = 0; x < this->width_; x += 1) {
                this->render_cell(x, y, this->grid_[y][x], last_fg, last_bg);
            }
        }

        this->full_redraw_ = false;
    } else if (!this->dirty_.empty()) {
        std::optional<Rgb> last_fg{}, last_bg{};
        for (const auto& [x, y]: this->dirty_) { this->render_cell(x, y, this->grid_[y][x], last_fg, last_bg); }
    }
    this->dirty_.clear();

    // Reset style after rendering to avoid side effects on the terminal.
    ansi::reset_style(this->back_buffer_);

    ansi::move_to(this->back_buffer_, this->cur_.row_, this->cur_.col_);
    ansi::cursor(this->back_buffer_, this->cur_style_);
    if (this->cur_style_ != ansi::CursorStyle::HIDDEN) { ansi::show_cursor(this->back_buffer_); }

    this->flush(tty);
}

void Display::render_cell(const std::size_t x, const std::size_t y, const Cell& cell, std::optional<Rgb>& last_fg,
                          std::optional<Rgb>& last_bg) {
    // Cells with length 0 won't be rendered, since nothing would be seen.
    if (cell.len_ == 0) { return; }

    ansi::move_to(this->back_buffer_, y + 1, x + 1);

    // Only write and update color if it changed.
    if (!last_fg.has_value() || *last_fg != cell.fg_) {
        ansi::rgb(this->back_buffer_, cell.fg_);
        last_fg = cell.fg_;
    }
    if (!last_bg.has_value() || *last_bg != cell.bg_) {
        ansi::rgb(this->back_buffer_, cell.bg_, false);
        last_bg = cell.bg_;
    }

    // Add the cell's Unicode codepoint.
    this->back_buffer_.append(cell.data_, cell.data_ + cell.len_);
}

void Display::flush(uv_tty_t* tty) {
    if (this->back_buffer_.empty()) { return; }

    // Double buffering.
    this->output_buffer_ = std::move(this->back_buffer_);

    this->back_buffer_.clear();
    this->back_buffer_.reserve(output_buffer_.size());

    // Write to stdout via libuv.
    this->is_writing_ = true;
    const uv_buf_t buf = uv_buf_init(this->output_buffer_.data(), this->output_buffer_.size());
    uv_write(&this->write_req_, reinterpret_cast<uv_stream_t*>(tty), &buf, 1, [](uv_write_t* req, int) {
        auto* self = static_cast<Display*>(req->data);
        self->is_writing_ = false;
    });
}
