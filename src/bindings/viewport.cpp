#include "../viewport.hpp"

#include <sol/sol.hpp>

#include "../document.hpp"

void Viewport::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        // Properties.
        "doc", &Viewport::doc_,
        "cursor", sol::property([](Viewport& self) { return &self.cur_; }),

        // Functions.
        "move_cursor", &Viewport::move_cursor,
        "toggle_gutter", [](Viewport& self) {
            self.gutter_ = !self.gutter_;
            self.adjust_viewport();
        },
        "set_mode_line", [](Viewport& self, const sol::protected_function& callback) { self.mode_line_renderer_ = callback; },
        "toggle_mode_line", [](Viewport& self) {
            self.mode_line_ = !self.mode_line_;
            self.adjust_viewport();
        },
        "scroll_up", [](Viewport& self, const std::size_t n = 1) { self.scroll_up(n); },
        "scroll_down", [](Viewport& self, const std::size_t n = 1) { self.scroll_down(n); },
        "scroll_left", [](Viewport& self, const std::size_t n = 1) { self.scroll_left(n); },
        "scroll_right", [](Viewport& self, const std::size_t n   = 1) { self.scroll_right(n); });
    // clang-format on
}
