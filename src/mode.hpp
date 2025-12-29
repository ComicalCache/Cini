#ifndef MODE_HPP_
#define MODE_HPP_

#include <functional>

#include <sol/sol.hpp>

#include "face.hpp"
#include "key.hpp"
#include "replacement.hpp"

struct Editor;

/// A command is a function that modifies the Editor state.
using Command = std::function<void(Editor&)>;
using CatchAllCommand = std::function<bool(Editor&, Key)>;

namespace mode {
    using Keymap = std::unordered_map<Key, Command>;
}

/// A mode that specifies a Keymap and Faces.
/// This should only be created by Editor::get_mode.
struct Mode {
public:
    std::string name_{};
    mode::Keymap keymap_{};
    /// Catchall command. If it is not NO-OP it MUST return true.
    CatchAllCommand catch_all_{};

    /// Character replacement during rendering.
    replacement::ReplacementMap replacements_{};
    face::FaceMap faces_{};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(Editor& editor, sol::table& core, sol::table& keybind);
};

#endif
