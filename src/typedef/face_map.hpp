#ifndef FACE_MAP_HPP_
#define FACE_MAP_HPP_

#include "string_hash.hpp"
#include "../face.hpp"

using FaceMap = std::unordered_map<std::string, Face, StringHash, std::equal_to<>>;

#endif
