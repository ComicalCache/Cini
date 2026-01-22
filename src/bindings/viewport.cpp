#include "../viewport.hpp"

#include <sol/sol.hpp>

// Include required because viewport.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.

void Viewport::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        /* Properties. */
        /// The Document of the Viewport.
        "doc", &Viewport::doc_,
        /// The Cursor of the Viewport.
        "cursor", sol::property([](const Viewport& self) -> Cursor { return self.cur_; }),

        /* Functions. */
        /// Moves the Cursor using a Cursor move function.
        "move_cursor", &Viewport::move_cursor,
        /// Toggles the gutter.
        "toggle_gutter", [](Viewport& self) -> void {
            self.gutter_ = !self.gutter_;
            self.adjust_viewport();
        },
        /// Configures the Mode Line. This can hold user-defined properties.
        /// It is configured using a table with
        ///     - { text = "...", face = "..." } for content
        ///     - { spacer = true, face = "..." } for spacers
        ///         (face can be a face object or name of a face)
        "set_mode_line", [](Viewport& self, const sol::protected_function& callback) -> void {
            self.mode_line_callback_ = callback;
        },
        /// Toggles the Mode Line.
        "toggle_mode_line", [](Viewport& self) -> void {
            self.mode_line_ = !self.mode_line_;
            self.adjust_viewport();
        },
        /// Moves the Viewport up.
        "scroll_up", [](Viewport& self, const std::size_t n) -> void { self.scroll_up(n); },
        /// Moves the Viewport down.
        "scroll_down", [](Viewport& self, const std::size_t n) -> void { self.scroll_down(n); },
        /// Moves the Viewport to the left.
        "scroll_left", [](Viewport& self, const std::size_t n) -> void { self.scroll_left(n); },
        /// Moves the Viewport to the right.
        "scroll_right", [](Viewport& self, const std::size_t n) -> void { self.scroll_right(n); },
        /// Sets the get_face callback to retreive a face by name.
        "set_get_face", [](Viewport& self, const sol::protected_function& callback) -> void {
            self.get_face_callback_ = callback;
        });
    // clang-format on
}
