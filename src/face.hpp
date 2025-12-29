#ifndef FACES_HPP_
#define FACES_HPP_

#include <optional>
#include <unordered_map>

#include "rgb.hpp"
#include "string_hash.hpp"

/// Color face.
struct Face {
public:
    std::optional<Rgb> fg_{};
    std::optional<Rgb> bg_{};

public:
    /// Merges two faces, overriding this with other's colors.
    void merge(const Face& other);
};

namespace face {
    using FaceMap = std::unordered_map<std::string, Face, StringHash, std::equal_to<>>;
}

#endif
