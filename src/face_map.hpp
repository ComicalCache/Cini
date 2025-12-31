#ifndef FACE_MAP_HPP_
#define FACE_MAP_HPP_

#include "face.hpp"
#include "string_hash.hpp"

using FaceMap = std::unordered_map<std::string, Face, StringHash, std::equal_to<>>;

#endif
