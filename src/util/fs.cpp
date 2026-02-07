#include "fs.hpp"

#include <fstream>

namespace fs {
    auto read_file(const std::filesystem::path& path) -> std::optional<std::string> {
        std::ifstream file(path);

        if (!file.is_open()) { return std::nullopt; }

        file.seekg(0, std::ios::end);
        std::string buffer{};
        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::ptrdiff_t>(buffer.size()));

        return buffer;
    }

    auto
    write_file(const std::filesystem::path& path, const std::string_view contents, const std::ios_base::openmode mode)
        -> bool {
        std::ofstream file(path, mode);

        if (!file.is_open()) { return false; }
        file.write(contents.data(), static_cast<std::ptrdiff_t>(contents.size()));

        return file.good();
    }

    auto absolute(const std::filesystem::path& path) -> std::optional<std::filesystem::path> {
        std::error_code err{};
        const auto new_path = std::filesystem::absolute(path, err);
        return err ? path : new_path;
    }

    auto equal(const std::filesystem::path& p1, const std::filesystem::path& p2) -> bool {
        std::error_code err{};
        return std::filesystem::equivalent(p1, p2, err);
    }

} // namespace fs
