#ifndef CURSOR_HPP_
#define CURSOR_HPP_

#include <sol/sol.hpp>

#include "types/position.hpp"

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

    /// Gets the character under the cursor.
    [[nodiscard]]
    auto current_char(const Document& doc) const -> std::size_t;
    /// Steps forward one character.
    auto step_forward(const Document& doc) -> bool;
    /// Steps backward one character.
    auto step_backward(const Document& doc) -> bool;
    /// Steps forward one character.
    [[nodiscard]]
    auto peek_forward(const Document& doc) -> std::optional<std::size_t>;
    /// Steps backward one character.
    [[nodiscard]]
    auto peek_backward(const Document& doc) -> std::optional<std::size_t>;

    /// Moves to a specific point in the Document the Cursor is pointing at.
    void point(const Document& doc, std::size_t point);
    /// Returns the point in the Document the Cursor is pointing at.
    [[nodiscard]]
    auto point(const Document& doc) const -> std::size_t;

    /// Jumps to the beginning of the line.
    void _jump_to_beginning_of_line(const Document& doc);
    /// Jumps to the end of the line.
    void _jump_to_end_of_line(const Document& doc);
    /// Jumps to the beginning of the file.
    void _jump_to_beginning_of_file(const Document& doc);
    /// Jumps to the end of the file.
    void _jump_to_end_of_file(const Document& doc);

    /// Jumps to the next word.
    void _next_word(const Document& doc, std::size_t n);
    /// Jumps to the end of the next word.
    void _next_word_end(const Document& doc, std::size_t n);
    /// Jumps to the previous word.
    void _prev_word(const Document& doc, std::size_t n);
    /// Jumps to the end of the previous word.
    void _prev_word_end(const Document& doc, std::size_t n);

    /// Jumps to the next whitespace.
    void _next_whitespace(const Document& doc, std::size_t n);
    /// Jumps to the previous whitespace.
    void _prev_whitespace(const Document& doc, std::size_t n);

    /// Jumps to the next empty line.
    void _next_empty_line(const Document& doc, std::size_t n);
    /// Jumps to the previous empty line.
    void _prev_empty_line(const Document& doc, std::size_t n);

    /// Jumps to the matching opposite bracket (if exists).
    void _jump_to_matching_opposite(const Document& doc);

private:
    /// Updates the preferred column to the current position.
    void update_pref_col(const Document& doc, std::size_t tab_width);
};

namespace cursor {
    using move_fn = std::function<void(Cursor&, const Document&, std::size_t)>;
}

#endif
