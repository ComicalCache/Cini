#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <filesystem>
#include <optional>
#include <vector>

#include <sol/table.hpp>

#include "container/property_map.hpp"

struct RegexMatch;
struct DocumentBinding;

/// Generic opened document, optionally backed by a file.
struct Document : public std::enable_shared_from_this<Document> {
    friend DocumentBinding;

public:
    std::size_t point_{0};
    /// Backing file.
    std::optional<std::filesystem::path> path_;
    /// Document properties.
    sol::table properties_;
    /// Properties bound to text ranges.
    PropertyMap text_properties_{};

private:
    // TODO: replace std::string with a more performant structure (PieceTable, Rope).
    /// Document data.
    std::string data_{};
    /// Starting indices of lines.
    std::vector<std::size_t> line_indices_{};

public:
    explicit Document(std::optional<std::filesystem::path> path, sol::state& lua);

    /// Writes the contents to the underlying or new path.
    void save(std::optional<std::filesystem::path> path);

    /// Gets the number of lines of the document.
    [[nodiscard]]
    auto line_count() const -> std::size_t;
    /// Gets the size of the document.
    [[nodiscard]]
    auto size() const -> std::size_t;

    /// Inserts data into the document at pos.
    void insert(std::size_t pos, std::string_view data);
    /// Removes data from start to end from the document.
    void remove(std::size_t start, std::size_t end);
    /// Clears the document.
    void clear();
    /// Replaces data from start to end with new_data.
    void replace(std::size_t start, std::size_t end, std::string_view new_data);

    /// Gets the nth line of the document.
    [[nodiscard]]
    auto line(std::size_t nth) const -> std::string_view;
    /// Gets a slice of data from start point to end point.
    [[nodiscard]]
    auto slice(std::size_t start, std::size_t end) const -> std::string_view;

    /// Searches the entire Document for a pattern.
    [[nodiscard]]
    auto search(std::string_view pattern) const -> std::vector<RegexMatch>;
    /// Searches the Document starting from the current point for a pattern.
    [[nodiscard]]
    auto search_forward(std::string_view pattern) const -> std::vector<RegexMatch>;
    /// Searches the Document backwards from the current point for a pattern.
    [[nodiscard]]
    auto search_backward(std::string_view pattern) const -> std::vector<RegexMatch>;

    /// Add or update a property on a text range.
    void add_text_property(std::size_t start, std::size_t end, std::string key, sol::object value);
    /// Remove all matching properties in the given range.
    void remove_text_property(std::size_t start, std::size_t end, std::string_view key);
    /// Remove all or matching properties.
    void clear_text_properties(const sol::optional<std::string_view>& key = sol::nullopt);
    /// Optimizes properties by merging overlapping properties.
    void optimize_text_properties(std::string_view key);

    [[nodiscard]]
    auto get_text_property(std::size_t pos, std::string_view key) const -> sol::object;
    [[nodiscard]]
    auto get_text_properties(std::size_t pos, sol::state& lua) const -> sol::table;
    [[nodiscard]]
    auto get_raw_text_property(std::size_t pos, std::string_view key) const -> const Property*;

private:
    void build_line_indices();
    void update_line_indices_on_insert(std::size_t pos, std::string_view data);
    void update_line_indices_on_remove(std::size_t start, std::size_t end);
};

#endif
