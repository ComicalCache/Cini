#ifndef FACE_CACHE_HPP_
#define FACE_CACHE_HPP_

#include <vector>

#include "../types/face.hpp"
#include "../types/property.hpp"

struct Document;

/// Cache to reduce Lua calls for resolving Faces.
struct FaceCache {
public:
    /// Face found after last call to FaceCache::update.
    sol::optional<Face> face_{};

private:
    /// Properties to scan for Faces.
    const std::vector<Property>& properties_;
    /// Current found Property with a Face.
    std::vector<Property>::const_iterator curr_;
    std::size_t curr_end_{0};

public:
    explicit FaceCache(std::size_t idx, const Document& doc);

    /// Updates face_ to the face at the current index.
    void update(std::size_t idx, const std::function<sol::optional<Face>(std::string_view)>& get_face);
};

#endif
