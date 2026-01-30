#include "../cursor.hpp"

#include "../document.hpp"

void Cursor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Cursor>("Cursor",
        /* Properties. */
        "row", sol::property([](const Cursor& self) -> std::size_t { return self.pos_.row_; }),
        "col", sol::property([](const Cursor& self) -> std::size_t { return self.pos_.col_; }),

        /* Functions. */
        "up", &Cursor::up,
        "down", &Cursor::down,
        "left", &Cursor::left,
        "right", &Cursor::right,

        "step_forward", &Cursor::step_forward,
        "step_backward", &Cursor::step_backward,
        "peek_forward", &Cursor::peek_forward,
        "peek_backward", &Cursor::peek_backward,
        "move_to", [](Cursor& self, const Document& doc, const std::size_t point) -> void { self.point(doc, point); },
        "point", [](const Cursor& self, const Document& doc) -> std::size_t { return self.point(doc); },

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
