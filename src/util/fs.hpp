#ifndef FS_HPP_
#define FS_HPP_

#include <filesystem>
#include <optional>
#include <string>

namespace fs {
    /// Reads a file and returns it contents on success.
    auto read_file(const std::filesystem::path& path) -> std::optional<std::string>;

    /// Writes a string to a file.
    auto write_file(const std::filesystem::path& path, std::string_view contents, std::ios_base::openmode mode) -> bool;
} // namespace fs

#endif
