#include "../cursor.hpp"

#include <sol/sol.hpp>

#include "../document.hpp"

void Cursor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Cursor>("Cursor",
        /* Properties. */
        /// Physical row in the Document.
        "row", sol::property([](const Cursor& self) { return self.pos_.row_; }),
        /// Physical column in the Document.
        "col", sol::property([](const Cursor& self) { return self.pos_.col_; }),

        /* Functions. */
        /// Moves the Cursor up one row.
        "up", &Cursor::up,
        /// Moves the Cursor down one row.
        "down", &Cursor::down,
        /// Moves the Cursor left by one utf8 character.
        "left", &Cursor::left,
        /// Moves the Cursor right by one utf8 character.
        "right", &Cursor::right,

        /// Moves the Cursor to the next utf8 character.
        "step_forward", &Cursor::step_forward,
        /// Moves the Cursor to the previous utf8 character.
        "step_backward", &Cursor::step_backward,
        /// Returns the utf8 code-point of the next utf8 character.
        "peek_forward", &Cursor::peek_forward,
        /// Returns the utf8 code-point of the previous utf8 character.
        "peek_backward", &Cursor::peek_backward,
        /// Moves the Cursor to specific point in the Document.
        "move_to", [](Cursor& self, const Document& doc, const std::size_t point) { self.point(doc, point); },
        /// Returns the point the Cursor in the Document is pointing at.
        "point", [](const Cursor& self, const Document& doc) { return self.point(doc); },

        // Predefined movement functions for performance.
        "_jump_to_beginning_of_line", &Cursor::_jump_to_beginning_of_line,
        "_jump_to_end_of_line", &Cursor::_jump_to_end_of_line,
        "_jump_to_beginning_of_file", &Cursor::_jump_to_beginning_of_file,
        "_jump_to_end_of_file", &Cursor::_jump_to_end_of_file,
        "_next_word", &Cursor::_next_word,
        "_next_word_end", &Cursor::_next_word_end,
        "_prev_word", &Cursor::_prev_word,
        "_prev_word_end", &Cursor::_prev_word_end,
        "_next_whitespace", &Cursor::_next_whitespace,
        "_prev_whitespace", &Cursor::_prev_whitespace,
        "_next_empty_line", &Cursor::_next_empty_line,
        "_prev_empty_line", &Cursor::_prev_empty_line,
        "_jump_to_matching_opposite", &Cursor::_jump_to_matching_opposite);
    // clang-format on
}
