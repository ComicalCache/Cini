#ifndef KEY_MAP_HPP_
#define KEY_MAP_HPP_

#include "key_hash.hpp"
#include "command.hpp"

using Keymap = std::unordered_map<Key, Command>;

#endif
