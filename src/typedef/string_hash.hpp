#ifndef STRING_HASH_HPP_
#define STRING_HASH_HPP_

#include <string_view>

/// String Hasher.
struct StringHash {
public:
    /// C++20 magic marker.
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const;
};

#endif
