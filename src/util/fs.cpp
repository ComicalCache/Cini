#include "fs.hpp"

#include <fstream>

namespace fs {
    std::optional<std::string> read_file(const std::filesystem::path& path) {
        std::ifstream file(path);

        if (!file.is_open()) { return std::nullopt; }

        file.seekg(0, std::ios::end);
        std::string buffer{};
        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::ptrdiff_t>(buffer.size()));

        return buffer;
    }

    bool
    write_file(const std::filesystem::path& path, const std::string_view contents, const std::ios_base::openmode mode) {
        std::ofstream file(path, mode);

        if (!file.is_open()) { return false; }
        file.write(contents.data(), static_cast<std::ptrdiff_t>(contents.size()));

        return file.good();
    }
} // namespace fs
