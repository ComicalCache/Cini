#ifndef MODE_HPP_
#define MODE_HPP_

#include <functional>

#include "key.hpp"

struct Editor;

/// A command is a function that modifies the Editor state.
using Command = std::function<void(Editor&)>;

using Keymap = std::unordered_map<Key, Command>;

/// A mode that specifies a Keymap.
struct Mode {
public:
    std::string name_;
    Keymap keymap_;
};

#endif
