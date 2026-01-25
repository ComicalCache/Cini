#include "../../document.hpp"

#include "../../editor.hpp"
#include "../../util/assert.hpp"

void Document::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Document>("Document",
        /* Properties. */
        /// The current point in the Document.
        "point", sol::readonly(&Document::point_),
        /// Document attached properties. This can hold user-defined properties.
        /// The following properties serve a specific function:
        ///     - "ws": whitespace replacement character
        ///     - "nl": newline replacement character
        ///     - "tab": tab replacement character
        ///     - "tab_width": width of the tab character
        ///     - "major_mode": Major Mode of the Document
        ///     - "minor_modes": stack of Minor Modes of the Document
        ///     - "minor_mode_override": Mode to (temporary) override all Minor Modes
        "properties", &Document::properties_,
        /// The backing file of the Document.
        "path", sol::property([](const Document& self) -> std::optional<std::string> {
            return self.path_.transform([](const std::filesystem::path& path) -> std::string { return path.string(); });
        }),
        /// The size in bytes of the data in the Document.
        "size", sol::property([](const Document& self) -> std::size_t { return self.data_.size(); }),

        /* Functions. */
        /// Sets the point of the Document.
        "set_point", [](Document& self, const std::size_t point) -> void {
            ASSERT(point <= self.data_.size(), "");

            self.point_ = point;
        },
        /// Inserts data at a point into the Document.
        "insert", &Document::insert,
        /// Removes a range of data from the Document.
        "remove", &Document::remove,
        /// Clears the entire Document.
        "clear", &Document::clear,
        /// Replaces a range from start to end with data.
        "replace", &Document::replace,
        /// Returns the nth line of the Document.
        "line", &Document::line,
        /// Returns a range of data from start to end form.
        "slice", &Document::slice,
        /// Returns matches for a regex pattern over the entire Document.
        "search", &Document::search,
        /// Returns matches for a regex pattern starting at the current point.
        "search_forward", &Document::search_forward,
        /// Returns matches for a regex pattern up to the current point.
        "search_backward", &Document::search_backward,
        /// Adds properties to a text range from start to end. This can hold user-defined properties.
        /// The following properties serve a specific function:
        ///     - "face": color face of the text range
        ///     - "replacement": replacement string that is displayed instead
        ///     - "keymap": keybinds that are set for the text range
        "add_text_property", &Document::add_text_property,
        /// Removes propreties from a text range.
        "remove_text_property", &Document::remove_text_property,
        /// Removes all (matching) text properties from the Document.
        "clear_text_properties", &Document::clear_text_properties,
        /// Merges matching overlapping text properties.
        "optimize_text_properties", &Document::optimize_text_properties,
        /// Returns the matching property at a point.
        "get_text_property", &Document::get_text_property,
        /// Returns a table of all properties at a point.
        "get_text_properties", [](const Document& self, const std::size_t pos, const sol::this_state L) -> sol::table {
            return self.get_text_properties(pos, L);
        });
    // clang-format on
}
