#ifndef REPLACEMENT_MAP_HPP_
#define REPLACEMENT_MAP_HPP_

#include "replacement.hpp"
#include "string_hash.hpp"

using ReplacementMap = std::unordered_map<std::string, Replacement, StringHash, std::equal_to<>>;

#endif
