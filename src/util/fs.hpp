#ifndef FS_HPP_
#define FS_HPP_

#include <filesystem>
#include <optional>
#include <string>

namespace fs {
    /// Reads a file and returns it contents on success.
    std::optional<std::string> read_file(const std::filesystem::path& path);

    /// Writes a string to a file.
    bool write_file(const std::filesystem::path& path, std::string_view contents, std::ios_base::openmode mode);
} // namespace fs

#endif
