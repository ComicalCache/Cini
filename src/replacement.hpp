#ifndef REPLACEMENT_HPP_
#define REPLACEMENT_HPP_

#include <string>
#include <unordered_map>

#include "string_hash.hpp"

/// Character replacement.
struct Replacement {
public:
    std::string txt{};
    std::string face{};
};

namespace replacement {
    using ReplacementMap = std::unordered_map<std::string, Replacement, StringHash, std::equal_to<>>;
}

#endif
