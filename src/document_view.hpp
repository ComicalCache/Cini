#ifndef DOCUMENT_VIEW_HPP_
#define DOCUMENT_VIEW_HPP_

#include <memory>
#include <string>
#include <string_view>

#include <sol/state.hpp>
#include <sol/table.hpp>

#include "container/property_map.hpp"
#include "cursor.hpp"
#include "util/instance_tracker.hpp"

struct Document;

/// A DocumentView acts as a middle-man between Viewport and Document allowing for view specific Document settings while
/// keeping the Viewport agnostic to the underlying data.
struct DocumentView : public InstanceTracker<DocumentView>, public std::enable_shared_from_this<DocumentView> {
public:
    std::shared_ptr<Document> doc_;

    Cursor cur_{};

    sol::table properties_;
    PropertyMap view_properties_{};

public:
    DocumentView(std::shared_ptr<Document> doc, sol::state& lua);

    DocumentView(const DocumentView&) = delete;
    auto operator=(const DocumentView&) -> DocumentView& = delete;
    DocumentView(DocumentView&&) = delete;
    auto operator=(DocumentView&&) -> DocumentView& = delete;

    /// Moves the cursor, returning if the cursor moved.
    auto move_cursor(const cursor::move_fn& move_fn, std::size_t n) -> bool;
    /// Resets the cursor to the beginning of the Document.
    void reset_cursor();

    /// Add or update a view property on a text range.
    void add_view_property(std::size_t start, std::size_t end, const std::string& key, sol::object value);
    /// Remove all matching view properties in the given range.
    void remove_view_property(std::size_t start, std::size_t end, std::string_view key);
    /// Remove all or matching view properties.
    void clear_view_properties(const sol::optional<std::string>& key = sol::nullopt);
    /// Optimizes view properties by merging overlapping properties.
    void optimize_view_properties(std::string_view key);

    [[nodiscard]]
    auto get_view_property(std::size_t pos, std::string_view key) const -> sol::object;
    [[nodiscard]]
    auto get_view_properties(std::size_t pos, sol::state& lua) const -> sol::table;
    [[nodiscard]]
    auto get_all_view_properties(std::string_view key, sol::state& lua) const -> sol::table;
    [[nodiscard]]
    auto get_raw_view_property(std::size_t pos, std::string_view key) const -> const Property*;

    [[nodiscard]]
    auto clone() const -> std::shared_ptr<DocumentView>;
};

#endif
