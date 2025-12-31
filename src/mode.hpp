#ifndef MODE_HPP_
#define MODE_HPP_

#include "command.hpp"
#include "face_map.hpp"
#include "key_map.hpp"
#include "replacement_map.hpp"
#include "syntax_rule.hpp"

struct Editor;

/// A mode that specifies a Keymap and Faces.
/// This should only be created by Editor::get_mode.
struct Mode {
public:
    std::string name_{};
    Keymap keymap_{};
    /// Catchall command. If it is not NO-OP it MUST return true.
    CatchAllCommand catch_all_{};

    /// Character replacement during rendering.
    ReplacementMap replacements_{};
    FaceMap faces_{};

    /// SyntaxRules are processed front to back, order matters.
    std::vector<SyntaxRule> syntax_rules_{};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(Editor& editor, sol::table& core, sol::table& keybind);
};

#endif
