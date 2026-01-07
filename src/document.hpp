#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <filesystem>
#include <optional>
#include <vector>

#include <sol/sol.hpp>

struct Editor;
struct Mode;

/// Generic opened document, optionally backed by a file.
struct Document {
public:
    static sol::protected_function open_callback_;

    /// Major Mode for the Document.
    std::shared_ptr<Mode> major_mode_{nullptr};
    /// Local Minor Modes of the Document. Evaluated in stack order.
    std::vector<std::shared_ptr<Mode>> minor_modes_{};

    std::size_t tab_width_{4};

private:
    // TODO: replace std::string with a more performant structure (PieceTable, Rope).
    /// Document data.
    std::string data_{};
    /// Backing file.
    std::optional<std::filesystem::path> path_;

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(Editor& editor, sol::table& core);

    /// Sets the callback called on opening a new Document.
    static void set_open_callback(const sol::protected_function& open_callback);

    explicit Document(std::optional<std::filesystem::path> path);

    /// Gets an immutable view of the document's data.
    [[nodiscard]] std::string_view data() const;

    /// Gets the number of lines of the document.
    [[nodiscard]] std::size_t line_count() const;
    /// Gets an immutable view of the nth line of the document.
    [[nodiscard]] std::string_view line(std::size_t nth) const;

    /// Inserts data into the document at pos.
    void insert(std::size_t pos, std::string_view data);
    /// Removes data of len at pos from the document.
    void remove(std::size_t pos, std::size_t n);
    /// Clears the document.
    void clear();
};

#endif
