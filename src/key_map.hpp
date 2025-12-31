#ifndef KEY_MAP_HPP_
#define KEY_MAP_HPP_

#include "command.hpp"
#include "key_hash.hpp"

using Keymap = std::unordered_map<Key, Command>;

#endif
