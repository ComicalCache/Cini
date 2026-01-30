#include "../viewport.hpp"

// Include required because viewport.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.

void Viewport::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        /* Properties. */
        "doc", sol::readonly(&Viewport::doc_),
        "cursor", sol::readonly(&Viewport::cur_),

        /* Functions. */
        "change_document", &Viewport::change_document,
        "move_cursor", &Viewport::move_cursor,
        "toggle_gutter", [](Viewport& self) -> void {
            self.gutter_ = !self.gutter_;
            self.adjust_viewport();
        },
        "set_mode_line", [](Viewport& self, const sol::protected_function& callback) -> void {
            self.mode_line_callback_ = callback;
        },
        "toggle_mode_line", [](Viewport& self) -> void {
            self.mode_line_ = !self.mode_line_;
            self.adjust_viewport();
        },
        "scroll_up", [](Viewport& self, const std::size_t n) -> void { self.scroll_up(n); },
        "scroll_down", [](Viewport& self, const std::size_t n) -> void { self.scroll_down(n); },
        "scroll_left", [](Viewport& self, const std::size_t n) -> void { self.scroll_left(n); },
        "scroll_right", [](Viewport& self, const std::size_t n) -> void { self.scroll_right(n); });
    // clang-format on
}
