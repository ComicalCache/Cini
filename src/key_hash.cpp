#include "key_hash.hpp"

std::size_t std::hash<Key>::operator()(const Key& k) const noexcept {
    return std::hash<std::size_t>{}(k.code_) ^ std::hash<std::size_t>{}(static_cast<std::size_t>(k.mod_) << 1);
}
