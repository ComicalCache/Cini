#ifndef CURSOR_HPP_
#define CURSOR_HPP_

#include <functional>
#include <optional>

#include "types/position.hpp"

struct Document;

/// A Cursor is the virtual index into a Document. It manages the byte offsets into the Document as well as a preferred
/// column to keep the Cursor in one column while scrolling.
///
/// It is assumed that the Cursor's corresponding Document is not changed without calling
/// Cursor::point(Document&, std::size_t) prior to correctly initialize the position. Failure to do so will lead to UB
/// and crashes.
struct Cursor {
public:
    /// Cursor position (byte indices).
    Position pos_{};
    /// Preferred column (logical index).
    std::size_t pref_col_{0};

public:
    /// Moves the cursor n lines up.
    void up(const Document& doc, std::size_t n);
    /// Moves the cursor n lines down.
    void down(const Document& doc, std::size_t n);
    /// Moves the cursor n characters to the left, wrapping lines.
    void left(const Document& doc, std::size_t n);
    /// Moves the cursor n characters to the right, wrapping lines.
    void right(const Document& doc, std::size_t n);

    /// Gets the character under the cursor.
    [[nodiscard]]
    auto current_char(const Document& doc) const -> std::size_t;
    /// Steps forward one character.
    auto step_forward(const Document& doc) -> bool;
    /// Steps backward one character.
    auto step_backward(const Document& doc) -> bool;
    /// Gets the next character.
    [[nodiscard]]
    auto peek_forward(const Document& doc) -> std::optional<std::size_t>;
    /// Gets the previous character.
    [[nodiscard]]
    auto peek_backward(const Document& doc) -> std::optional<std::size_t>;

    /// Moves the Cursor to a specific point in the Document.
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

    /// Jumps to the nth next word.
    void _next_word(const Document& doc, std::size_t n);
    /// Jumps to the nth end of the next word.
    void _next_word_end(const Document& doc, std::size_t n);
    /// Jumps to the nth previous word.
    void _prev_word(const Document& doc, std::size_t n);
    /// Jumps to the nth end of the previous word.
    void _prev_word_end(const Document& doc, std::size_t n);

    /// Jumps to the nth next whitespace.
    void _next_whitespace(const Document& doc, std::size_t n);
    /// Jumps to the nth previous whitespace.
    void _prev_whitespace(const Document& doc, std::size_t n);

    /// Jumps to the nth next empty line.
    void _next_empty_line(const Document& doc, std::size_t n);
    /// Jumps to the nth previous empty line.
    void _prev_empty_line(const Document& doc, std::size_t n);

    /// Jumps to the matching opposite bracket (if exists).
    void _jump_to_matching_opposite(const Document& doc);

private:
    /// Updates the preferred column to the current position.
    void update_pref_col(const Document& doc, std::size_t tab_width);
};

namespace cursor {
    /// A generic movement function taking the Document and amount of times repeating the operation.
    using move_fn = std::function<void(Cursor&, const Document&, std::size_t)>;
} // namespace cursor

#endif
