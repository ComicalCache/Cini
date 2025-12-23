#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include <optional>
#include <string>
#include <vector>

#include <uv.h>

#include "cell.hpp"
#include "cursor.hpp"
#include "rgb.hpp"

/// Display abstraction of the terminal.
class Display {
private:
    /// Write request to handle libuv stdout.
    uv_write_t write_req_{};
    /// Flag to control writing through libuv.
    bool is_writing_{false};

    std::size_t width_{0}, height_{0};
    /// Full redraw flag for specific logic.
    bool full_redraw_{true};

    /// Terminal cursor position.
    Cursor cur_{};
    /// Cell grid of the terminal.
    std::vector<std::vector<Cell>> grid_{};
    /// Dirty cells that need to be written to the terminal.
    std::vector<std::pair<int, int>> dirty_{};

    /// Double buffer back buffer.
    std::string back_buffer_{};
    /// Double buffer output buffer.
    std::string output_buffer_{};

public:
    Display();

    /// Resizes the Display.
    void resize(std::size_t width, std::size_t height);
    /// Updates a Cell.
    void update(std::size_t x, std::size_t y, const Cell& cell);
    /// Sets the Cursor (zero indexed).
    void cursor(std::size_t row, std::size_t col);
    /// Renders the Display to stdout.
    void render(uv_tty_t* tty);

private:
    /// Writes the ANSI sequences to render a Cell to the buffer (zero indexed).
    void render_cell(std::size_t x, std::size_t y, const Cell& cell, std::optional<Rgb>& last_fg,
                     std::optional<Rgb>& last_bg);
    /// Flushes the buffer to stdout via libuv.
    void flush(uv_tty_t* tty);
};

#endif
