#include "mode.hpp"

#include "editor.hpp"

void Mode::init_bridge(Editor& editor, sol::table& core, sol::table& keybind) {
    // clang-format off
    // Internal bind function. Should not be used by end-users directly as it only handles single sequence keybinds.
    keybind.set_function(
        "__bind", [&editor](const std::string_view mode, const std::string_view key_str, const sol::function& cmd) {
            if (Key key{0, key::Mod::NONE}; Key::try_parse_string(key_str, key)) {
                auto cpp_cmd = [cmd](Editor& self) {
                    // TODO: log error.
                    if (const auto result = cmd(self); !result.valid()) { sol::error err = result; }
                };

                editor.get_mode(mode).keymap_[key] = cpp_cmd;
            }
        });

    core.new_usertype<Mode>("Mode",
        "name", &Mode::name_,
        "bind", [](Mode& self, const std::string& key, const sol::function& cmd) {
            if (Key k{0, key::Mod::NONE}; Key::try_parse_string(key, k)) {
                self.keymap_[k] = [cmd](Editor& editor) {
                    // TODO: handle error.
                    if (const auto res = cmd(editor); !res.valid()) { sol::error err = res; }
                };
            }
        },
        "bind_catch_all", [](Mode& self, const sol::function& cmd) {
            self.catch_all_ = [cmd](Editor& editor, Key key) {
                const auto res = cmd(editor, key);
                if (!res.valid() || res.get_type() != sol::type::boolean) {
                    sol::error err = res;
                    // TODO: handle error.
                    return false;
                }
                return res.get<bool>();
            };
        },
        "set_face", [](Mode& self, const std::string& name, const Face& face) { self.faces_[name] = face; },
        "set_replacement", [](Mode& self, const std::string& ch, const std::string& txt, const std::string& face) {
            self.replacements_[ch] = Replacement{txt, face};
        });
    // clang-format on
}
