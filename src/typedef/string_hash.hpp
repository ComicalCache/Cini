#ifndef STRING_HASH_HPP_
#define STRING_HASH_HPP_

#include <string_view>

/// String Hasher.
struct StringHash {
public:
    /// C++20 magic marker.
    using is_transparent = void;

    auto operator()(std::string_view sv) const -> std::size_t;
};

#endif
