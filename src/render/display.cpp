#include "display.hpp"

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
    for (auto& row: this->grid_) { row.resize(width, Cell(" ")); }

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

        std::optional<Rgb> last_fg{};
        std::optional<Rgb> last_bg{};
        std::optional<bool> last_bold{};
        std::optional<bool> last_italic{};
        std::optional<bool> last_underline{};
        std::optional<bool> last_strike{};
        for (auto y{0UZ}; y < this->height_; y += 1) {
            for (auto x{0UZ}; x < this->width_; x += 1) {
                this->render_cell(
                    x, y, this->grid_[y][x], last_fg, last_bg, last_bold, last_italic, last_underline, last_strike);
            }
        }

        this->full_redraw_ = false;
    } else if (!this->dirty_.empty()) {
        std::optional<Rgb> last_fg{};
        std::optional<Rgb> last_bg{};
        std::optional<bool> last_bold{};
        std::optional<bool> last_italic{};
        std::optional<bool> last_underline{};
        std::optional<bool> last_strike{};
        for (const auto& [x, y]: this->dirty_) {
            this->render_cell(
                x, y, this->grid_[y][x], last_fg, last_bg, last_bold, last_italic, last_underline, last_strike);
        }
    }
    this->dirty_.clear();

    // Reset style after rendering to avoid side effects on the terminal.
    ansi::reset_style(this->back_buffer_);

    ansi::move_to(this->back_buffer_, this->cur_.row_, this->cur_.col_);
    ansi::cursor(this->back_buffer_, this->cur_style_);
    if (this->cur_style_ != ansi::CursorStyle::HIDDEN) { ansi::show_cursor(this->back_buffer_); }

    this->flush(tty);
}

void Display::render_cell(
    const std::size_t x, const std::size_t y, const Cell& cell, std::optional<Rgb>& last_fg,
    std::optional<Rgb>& last_bg, std::optional<bool>& last_bold, std::optional<bool>& last_italic,
    std::optional<bool>& last_underline, std::optional<bool>& last_strikethrough) {
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

    // Only write and update style if it changed.
    if (!last_bold.has_value() || *last_bold != cell.bold_) {
        ansi::bold(this->back_buffer_, cell.bold_);
        last_bold = cell.bold_;
    }
    if (!last_italic.has_value() || *last_italic != cell.italic_) {
        ansi::italic(this->back_buffer_, cell.italic_);
        last_italic = cell.italic_;
    }
    if (!last_underline.has_value() || *last_underline != cell.underline_) {
        ansi::underline(this->back_buffer_, cell.underline_);
        last_underline = cell.underline_;
    }
    if (!last_strikethrough.has_value() || *last_strikethrough != cell.strikethrough_) {
        ansi::strikethrough(this->back_buffer_, cell.strikethrough_);
        last_strikethrough = cell.strikethrough_;
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
    const uv_buf_t buf = uv_buf_init(this->output_buffer_.data(), this->output_buffer_.size());
    uv_write(&this->write_req_, reinterpret_cast<uv_stream_t*>(tty), &buf, 1, [](uv_write_t* req, int) -> void {
        auto* self = static_cast<Display*>(req->data);
        self->is_writing_ = false;
    });
}
