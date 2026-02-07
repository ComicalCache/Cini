#ifndef FACES_HPP_
#define FACES_HPP_

#include <optional>

#include "rgb.hpp"

/// Faces define RGB colors of Cells drawn to the Display.
struct Face {
public:
    std::optional<Rgb> fg_{};
    std::optional<Rgb> bg_{};

public:
    /// Merges two faces, overriding this with other's colors.
    void merge(const Face& other);
};

#endif
