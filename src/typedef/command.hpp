#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include <functional>

struct Editor;
struct Key;

/// A command is a function that modifies the Editor state.
using Command = std::function<void(Editor&)>;
/// A catch-all command is a function that modifies the Editor state.
using CatchAllCommand = std::function<bool(Editor&, Key)>;

#endif
