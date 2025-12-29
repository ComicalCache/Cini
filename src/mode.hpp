#ifndef MODE_HPP_
#define MODE_HPP_

#include <functional>

#include "face.hpp"
#include "key.hpp"
#include "replacement.hpp"

struct Editor;

/// A command is a function that modifies the Editor state.
using Command = std::function<void(Editor&)>;

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
    std::function<bool(Editor&, Key)> catch_all_{};

    /// Character replacement during rendering.
    replacement::ReplacementMap replacements_{};
    face::FaceMap faces_{};
};

#endif
