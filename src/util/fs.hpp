#ifndef FS_HPP_
#define FS_HPP_

#include <filesystem>
#include <optional>
#include <string>

namespace fs {
    /// Reads a file and returns it contents on success.
    [[nodiscard]]
    auto read_file(const std::filesystem::path& path) -> std::optional<std::string>;
    /// Writes a string to a file.
    [[nodiscard]]
    auto write_file(const std::filesystem::path& path, std::string_view contents, std::ios_base::openmode mode) -> bool;

    /// Converts a path to an absolute path.
    [[nodiscard]]
    auto absolute(const std::filesystem::path& path) -> std::optional<std::filesystem::path>;
    /// Checks if two paths are equivalent.
    [[nodiscard]]
    auto equal(const std::filesystem::path& p1, const std::filesystem::path& p2) -> bool;
} // namespace fs

#endif
