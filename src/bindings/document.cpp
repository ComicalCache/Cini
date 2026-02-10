#include "document_binding.hpp"

#include "../document.hpp"
#include "../editor.hpp"
// Include required because editor.hpp forward declares RegexMatch.
#include "../types/regex_match.hpp" // IWYU pragma: keep.

void DocumentBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Document>("Document",
        /* Properties. */
        "point", sol::readonly(&Document::point_),
        "properties", &Document::properties_,
        "path", sol::property([](const Document& self) -> std::optional<std::string> {
            return self.path_.transform([](const std::filesystem::path& path) -> std::string { return path.string(); });
        }),
        "size", sol::property([](const Document& self) -> std::size_t { return self.data_.size(); }),

        /* Functions. */
        "set_point", &Document::set_point,
        "save", [](Document& self, std::optional<std::string_view> path) -> void { self.save(path); },
        "insert", &Document::insert,
        "remove", &Document::remove,
        "clear", &Document::clear,
        "replace", &Document::replace,
        "line", &Document::line,
        "slice", &Document::slice,
        "search", &Document::search,
        "search_forward", &Document::search_forward,
        "search_backward", &Document::search_backward,
        "add_text_property", &Document::add_text_property,
        "remove_text_property", &Document::remove_text_property,
        "clear_text_properties", &Document::clear_text_properties,
        "optimize_text_properties", &Document::optimize_text_properties,
        "get_text_property", &Document::get_text_property,
        "get_text_properties", [](const Document& self, const std::size_t pos) -> sol::table {
            return self.get_text_properties(pos, Editor::instance()->lua_);
        });
    // clang-format on
}
