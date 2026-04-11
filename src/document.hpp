#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <filesystem>
#include <optional>
#include <vector>

#include <sol/table.hpp>

#include "container/property_map.hpp"
#include "types/transaction.hpp"
#include "util/instance_tracker.hpp"

struct DocumentBinding;
struct DocumentView;
struct FaceCache;
struct Regex;
struct RegexMatch;
struct Position;

/// Documents serve as the central abstraction of data.
///
/// Data accesses should be done by the API of this class, if possible. Besides fetching lines, all position parameters
/// are byte indexed.
struct Document : public InstanceTracker<Document>, public std::enable_shared_from_this<Document> {
    friend DocumentBinding;
    friend FaceCache;

public:
    /// Backing file.
    std::optional<std::filesystem::path> path_;

    /// Document properties.
    sol::table properties_;
    /// Properties bound to text ranges.
    PropertyMap text_properties_{};

    bool modified_{false};

    std::vector<Transaction> undo_stack_{};
    std::vector<Transaction> redo_stack_{};
    Transaction active_transaction_{};
    bool recording_transaction_{false};
    bool applying_transaction_{false};

    std::vector<std::weak_ptr<DocumentView>> views_;

private:
    // TODO: replace std::string with a more performant structure (PieceTable, Rope).
    /// Document data.
    std::string data_{};
    /// Starting indices of lines.
    std::vector<std::size_t> line_indices_{};

public:
    Document(std::optional<std::filesystem::path> path, sol::state& lua);

    Document(const Document&) = delete;
    auto operator=(const Document&) -> Document& = delete;
    Document(Document&&) noexcept = default;
    auto operator=(Document&&) noexcept -> Document& = default;

    [[nodiscard]]
    auto views() -> std::vector<std::shared_ptr<DocumentView>>;

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

    /// Gets the byte beginning the nth line of the document.
    [[nodiscard]]
    auto line_begin_byte(std::size_t nth) const -> std::size_t;
    /// Gets the byte one after the end of the nth line of the document. This includes the newline character.
    [[nodiscard]]
    auto line_end_byte(std::size_t nth) const -> std::size_t;
    /// Gets the position struct from a byte offset.
    [[nodiscard]]
    auto position_from_byte(std::size_t byte) const -> Position;

    /// Searches the Document for a Regex pattern in a given range (defaults to the entire Document).
    [[nodiscard]]
    auto
    search(const Regex& regex, std::size_t start = 0, std::size_t end = std::numeric_limits<std::size_t>::max()) const
        -> std::vector<RegexMatch>;

    void begin_transaction(std::size_t point);
    void end_transaction(std::size_t point);
    /// Undos the last transaction, returing the position of the cursor after undoing.
    [[nodiscard]]
    auto undo() -> std::optional<std::size_t>;
    /// Redos the last undone transaction, return the position of the cursor after redoing.
    [[nodiscard]]
    auto redo() -> std::optional<std::size_t>;

    /// Add or update a property on a text range.
    void add_text_property(std::size_t start, std::size_t end, const std::string& key, sol::object value);
    /// Remove all matching properties in the given range.
    void remove_text_property(std::size_t start, std::size_t end, std::string_view key);
    /// Remove all or matching properties.
    void clear_text_properties(const sol::optional<std::string>& key = sol::nullopt);
    /// Optimizes properties by merging overlapping properties.
    void optimize_text_properties(std::string_view key);

    [[nodiscard]]
    auto get_text_property(std::size_t pos, std::string_view key) const -> sol::object;
    [[nodiscard]]
    auto get_text_properties(std::size_t pos, sol::state& lua) const -> sol::table;
    [[nodiscard]]
    auto get_all_text_properties(std::string_view key, sol::state& lua) const -> sol::table;
    [[nodiscard]]
    auto get_raw_text_property(std::size_t pos, std::string_view key) const -> const Property*;

private:
    void build_line_indices();
    void update_line_indices_on_insert(std::size_t pos, std::string_view data);
    void update_line_indices_on_remove(std::size_t start, std::size_t end);
};

#endif
