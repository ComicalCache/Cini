#ifndef CURSOR_HPP_
#define CURSOR_HPP_

#include <sol/sol.hpp>

#include "position.hpp"

struct Document;

/// Cursor on a Document.
struct Cursor {
public:
    /// Cursor position (byte indices).
    Position pos_{};

    /// Preferred column when scrolling text with different row lengths to "snap back" to previous column
    /// (logical index).
    std::size_t pref_col_{0};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    /// Moves the cursor n lines up.
    void up(const Document& doc, std::size_t n);
    /// Moves the cursor n lines down.
    void down(const Document& doc, std::size_t n);
    /// Moves the cursor n characters to the left.
    void left(const Document& doc, std::size_t n);
    /// Moves the cursor n characters to the right.
    void right(const Document& doc, std::size_t n);

    /// Returns the byte index in the Document the Cursor is pointing at.
    [[nodiscard]] std::size_t byte(const Document& doc) const;

private:
    /// Updates the preferred column to the current position.
    void update_pref_col(const Document& doc);
};

namespace cursor {
    using move_fn = std::function<void(Cursor&, const Document&, std::size_t)>;
}

#endif
