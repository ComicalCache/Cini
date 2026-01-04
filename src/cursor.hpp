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

    /// Moves to a specific position.
    void move_to(const Document& doc, Position pos);

    /// Jumps to the beginning of the line.
    void jump_to_beginning_of_line(const Document& doc);
    /// Jumps to the end of the line.
    void jump_to_end_of_line(const Document& doc);
    /// Jumps to the beginning of the file.
    void jump_to_beginning_of_file(const Document& doc);
    /// Jumps to the end of the file.
    void jump_to_end_of_file(const Document& doc);

    /// Jumps to the next word.
    void next_word(const Document& doc, std::size_t n);
    /// Jumps to the end of the next word.
    void next_word_end(const Document& doc, std::size_t n);
    /// Jumps to the previous word.
    void prev_word(const Document& doc, std::size_t n);
    /// Jumps to the end of the previous word.
    void prev_word_end(const Document& doc, std::size_t n);

    /// Jumps to the next whitespace.
    void next_whitespace(const Document& doc, std::size_t n);
    /// Jumps to the previous whitespace.
    void prev_whitespace(const Document& doc, std::size_t n);

    /// Jumps to the next empty line.
    void next_empty_line(const Document& doc, std::size_t n);
    /// Jumps to the previous empty line.
    void prev_empty_line(const Document& doc, std::size_t n);

    /// Jumps to the matching opposite bracket (if exists).
    void jump_to_matching_opposite(const Document& doc);

    /// Returns the byte index in the Document the Cursor is pointing at.
    [[nodiscard]] std::size_t byte(const Document& doc) const;

private:
    /// Updates the preferred column to the current position.
    void update_pref_col(const Document& doc);

    /// Gets the character under the cursor.
    [[nodiscard]] std::size_t current_char(const Document& doc) const;
    /// Steps forward one character.
    bool step_forward(const Document& doc);
    /// Steps backward one character.
    bool step_backward(const Document& doc);
};

namespace cursor {
    using move_fn = std::function<void(Cursor&, const Document&, std::size_t)>;
}

#endif
