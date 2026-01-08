#ifndef MODE_HPP_
#define MODE_HPP_

#include "typedef/command.hpp"
#include "types/syntax_rule.hpp"
#include "typedef/face_map.hpp"
#include "typedef/key_map.hpp"
#include "typedef/replacement_map.hpp"

struct Editor;

/// A mode that specifies a Keymap and Faces.
///
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
    Mode() = default;
    explicit Mode(std::string_view name);

    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(Editor& editor, sol::table& core, sol::table& keybind);
};

#endif
