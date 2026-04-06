#include "bindings.hpp"

// Include required because document_view.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.
#include "../document_view.hpp"
#include "../editor.hpp"

void DocumentViewBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<DocumentView>("DocumentView",
        /* Properties. */
        "doc", sol::readonly(&DocumentView::doc_),
        "cur", &DocumentView::cur_,
        "properties", &DocumentView::properties_,

        /* Functions. */
        "move_cursor", [](DocumentView& self, const sol::function& fn, std::size_t n) -> bool {
            return self.move_cursor([fn](Cursor& cur, const DocumentView& view, std::size_t n_steps) -> void {
                // Pass DocumentView pointer to avoid sol2 trying to copy uncopyable DocumentView.
                fn(cur, &view, n_steps);
            }, n);
        },
        "add_view_property", &DocumentView::add_view_property,
        "remove_view_property", &DocumentView::remove_view_property,
        "clear_view_properties", &DocumentView::clear_view_properties,
        "optimize_view_properties", &DocumentView::optimize_view_properties,
        "get_view_property", &DocumentView::get_view_property,
        "get_view_properties", [](const DocumentView& self, const std::size_t pos) -> sol::table {
            return self.get_view_properties(pos, Editor::instance()->lua_);
        },
        "get_all_view_properties", [](const DocumentView& self, const std::string_view key) -> sol::table {
            return self.get_all_view_properties(key, Editor::instance()->lua_);
        });
    // clang-format on
}
