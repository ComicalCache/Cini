#ifndef REPLACEMENT_MAP_HPP_
#define REPLACEMENT_MAP_HPP_

#include <unordered_map>

#include "string_hash.hpp"
#include "../types/replacement.hpp"

using ReplacementMap = std::unordered_map<std::string, Replacement, StringHash, std::equal_to<>>;

#endif
