#ifndef CURSOR_HPP_
#define CURSOR_HPP_

#include "position.hpp"

/// Cursor on the Display.
struct Cursor {
public:
    /// Cursor position.
    Position pos_{};

private:
    /// Preferred column when scrolling text with different row lengths to "snap back" to previous column.
    // std::size_t pref_col_{0};
};

#endif
