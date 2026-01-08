#include "../cursor.hpp"

#include <sol/sol.hpp>

#include "../document.hpp"

void Cursor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Cursor>("Cursor",
        // Properties.
        "row", sol::property([](const Cursor& self) { return self.pos_.row_; }),
        "col", sol::property([](const Cursor& self) { return self.pos_.col_; }),

        // Functions.
        "byte", &Cursor::byte,
        "move_to", [](Cursor& self, const Document& doc, const std::size_t row, const std::size_t col) {
          self.move_to(doc, Position{row, col});
        },
        "up", &Cursor::up,
        "down", &Cursor::down,
        "left", &Cursor::left,
        "right", &Cursor::right,
        "jump_to_beginning_of_line", &Cursor::jump_to_beginning_of_line,
        "jump_to_end_of_line", &Cursor::jump_to_end_of_line,
        "jump_to_beginning_of_file", &Cursor::jump_to_beginning_of_file,
        "jump_to_end_of_file", &Cursor::jump_to_end_of_file,
        "next_word", &Cursor::next_word,
        "next_word_end", &Cursor::next_word_end,
        "prev_word", &Cursor::prev_word,
        "prev_word_end", &Cursor::prev_word_end,
        "next_whitespace", &Cursor::next_whitespace,
        "prev_whitespace", &Cursor::prev_whitespace,
        "next_empty_line", &Cursor::next_empty_line,
        "prev_empty_line", &Cursor::prev_empty_line,
        "jump_to_matching_opposite", &Cursor::jump_to_matching_opposite);
    // clang-format on
}
