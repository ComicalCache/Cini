#include "bindings.hpp"

#include "../document.hpp"
#include "../document_view.hpp"
#include "../editor.hpp"
// Include required because document.hpp forward declares Regex.
#include "../regex.hpp" // IWYU pragma: keep.
// Include required because document.hpp forward declares RegexMatch.
#include "../types/regex_match.hpp" // IWYU pragma: keep.

void DocumentBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Document>("Document",
        /* Properties. */
        "properties", &Document::properties_,
        "path", sol::property([](const Document& self) -> std::optional<std::string> {
            return self.path_.transform([](const std::filesystem::path& path) -> std::string { return path.string(); });
        }),
        "size", sol::property([](const Document& self) -> std::size_t { return self.data_.size(); }),
        "modified", &Document::modified_,

        /* Functions. */
        "views", &Document::views,
        "save", [](Document& self, std::optional<std::string_view> path) -> void { self.save(path); },
        "insert", &Document::insert,
        "remove", &Document::remove,
        "clear", &Document::clear,
        "replace", &Document::replace,
        "line", &Document::line,
        "slice", &Document::slice,
        "line_begin_byte", &Document::line_begin_byte,
        "line_end_byte", &Document::line_end_byte,
        "position_from_byte", &Document::position_from_byte,
        "search", sol::overload(
            [](const Document& self, const Regex& regex, std::size_t start, std::size_t end)
                -> std::vector<RegexMatch> { return self.search(regex, start, end); },
            [](const Document& self, const Regex& regex) -> std::vector<RegexMatch> { return self.search(regex); }
        ),
        "begin_transaction", &Document::begin_transaction,
        "end_transaction", &Document::end_transaction,
        "undo", &Document::undo,
        "redo", &Document::redo,
        "add_text_property", &Document::add_text_property,
        "remove_text_property", &Document::remove_text_property,
        "clear_text_properties", &Document::clear_text_properties,
        "optimize_text_properties", &Document::optimize_text_properties,
        "get_text_property", &Document::get_text_property,
        "get_text_properties", [](const Document& self, const std::size_t pos) -> sol::table {
            return self.get_text_properties(pos, Editor::instance()->lua_);
        },
        "get_all_text_properties", [](const Document& self, const std::string_view key) -> sol::table {
            return self.get_all_text_properties(key, Editor::instance()->lua_);
        });
    // clang-format on
}
