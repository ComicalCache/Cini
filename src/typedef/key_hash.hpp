#ifndef KEY_HASH_HPP_
#define KEY_HASH_HPP_

#include "../key.hpp"

// Make key hashable to the Keymap.
template<>
struct std::hash<Key> {
    std::size_t operator()(const Key& k) const noexcept;
};

#endif
