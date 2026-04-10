#include "bindings.hpp"

#include "../viewport.hpp"
// Include required because viewport.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.
// Include required because viewport.hpp forward declares DocumentView.
#include "../document_view.hpp" // IWYU pragma: keep.

void ViewportBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        /* Properties. */
        "view", sol::readonly(&Viewport::view_),

        /* Functions. */
        "change_document_view", &Viewport::change_document_view,
        "adjust", &Viewport::adjust_viewport,
        "scroll_up", [](Viewport& self, const std::size_t n) -> void { self.scroll_up(n); },
        "scroll_down", [](Viewport& self, const std::size_t n) -> void { self.scroll_down(n); },
        "scroll_left", [](Viewport& self, const std::size_t n) -> void { self.scroll_left(n); },
        "scroll_right", [](Viewport& self, const std::size_t n) -> void { self.scroll_right(n); });
    // clang-format on
}
