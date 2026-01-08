#ifndef FACES_HPP_
#define FACES_HPP_

#include "types/rgb.hpp"

/// Color face.
struct Face {
public:
    std::optional<Rgb> fg_{};
    std::optional<Rgb> bg_{};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    /// Merges two faces, overriding this with other's colors.
    void merge(const Face& other);
};

#endif
