#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "mode.hpp"

/// Generic opened document, optionally backed by a file.
struct Document {
public:
    /// Major Mode for the Document.
    Mode major_mode_{};
    /// Local Minor Modes of the Document. Evaluated in stack order.
    std::vector<Mode> minor_modes_{};

private:
    // TODO: replace std::string with a more performant structure (PieceTable, Rope).
    /// Document data.
    std::string data_{};
    /// Backing file.
    std::optional<std::filesystem::path> path_;

public:
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
    void remove(std::size_t pos, std::size_t len);
};

#endif
