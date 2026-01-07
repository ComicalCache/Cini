#include "mode.hpp"

#include "editor.hpp"
#include "regex.hpp"
#include "util.hpp"

Mode::Mode(const std::string_view name) : name_{name} {}

void Mode::init_bridge(Editor& editor, sol::table& core, sol::table& keybind) {
    // clang-format off
    // Internal bind function. Should not be used by end-users directly as it only handles single sequence keybinds.
    keybind.set_function(
        "_bind", [&editor](const std::string_view mode, const std::string_view key_str, const sol::protected_function& cmd) {
            if (Key key{0, KeyMod::NONE}; Key::try_parse_string(key_str, key)) {
                auto cpp_cmd = [cmd](Editor& self) {
                    if (const auto result = cmd(self); !result.valid()) {
                        const sol::error err = result;
                        util::log::set_status_message(err.what());
                    }
                };

                editor.get_mode(mode)->keymap_[key] = cpp_cmd;
            }
        });

    core.new_usertype<Mode>("Mode",
        // Properties.
        "name", &Mode::name_,

        // Functions.
        "bind", [](Mode& self, const std::string& key, const sol::protected_function& cmd) {
            if (Key k{0, KeyMod::NONE}; Key::try_parse_string(key, k)) {
                self.keymap_[k] = [cmd](Editor& editor) {
                    if (const auto res = cmd(editor); !res.valid()) {
                        const sol::error err = res;
                        util::log::set_status_message(err.what());
                    }
                };
            }
        },
        "bind_catch_all", [](Mode& self, const sol::protected_function& cmd) {
            self.catch_all_ = [cmd](Editor& editor, Key key) {
                const auto res = cmd(editor, key);
                if (!res.valid() || res.get_type() != sol::type::boolean) {
                    const sol::error err = res;
                    util::log::set_status_message(err.what());
                    return false;
                }

                return res.get<bool>();
            };
        },
        "set_face", [](Mode& self, const std::string& name, const Face& face) { self.faces_[name] = face; },
        "set_replacement", [](Mode& self, const std::string& ch, const std::string& txt, const std::string& face) {
            self.replacements_[ch] = Replacement{txt, face};
        },
        "set_syntax", [](Mode& self, const std::string& pattern, const std::string& face) {
                self.syntax_rules_.push_back({std::make_shared<Regex>(pattern), face});
        });
    // clang-format on
}
