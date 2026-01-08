#include <sol/sol.hpp>

#include "cursor.hpp"
#include "direction.hpp"
#include "document.hpp"
#include "editor.hpp"
#include "face.hpp"
#include "key.hpp"
#include "mode.hpp"
#include "regex.hpp"
#include "util.hpp"
#include "viewport.hpp"
#include "window.hpp"

void Cursor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Cursor>("Cursor",
        // Properties.
        "row", sol::property([](const Cursor& self) { return self.pos_.row_; }),
        "col", sol::property([](const Cursor& self) { return self.pos_.col_; }),

        // Functions.
        "byte", &Cursor::byte,
        "move_to", [](Cursor& self, const Document& doc, const std::size_t row, const std::size_t col) {
          self.move_to(doc, Position{row, col});
        },
        "up", &Cursor::up,
        "down", &Cursor::down,
        "left", &Cursor::left,
        "right", &Cursor::right,
        "jump_to_beginning_of_line", &Cursor::jump_to_beginning_of_line,
        "jump_to_end_of_line", &Cursor::jump_to_end_of_line,
        "jump_to_beginning_of_file", &Cursor::jump_to_beginning_of_file,
        "jump_to_end_of_file", &Cursor::jump_to_end_of_file,
        "next_word", &Cursor::next_word,
        "next_word_end", &Cursor::next_word_end,
        "prev_word", &Cursor::prev_word,
        "prev_word_end", &Cursor::prev_word_end,
        "next_whitespace", &Cursor::next_whitespace,
        "prev_whitespace", &Cursor::prev_whitespace,
        "next_empty_line", &Cursor::next_empty_line,
        "prev_empty_line", &Cursor::prev_empty_line,
        "jump_to_matching_opposite", &Cursor::jump_to_matching_opposite);
    // clang-format on
}

void Document::init_bridge(Editor& editor, sol::table& core) {
    // clang-format off
    core.new_usertype<Document>("Document",
        // Properties.
        "major_mode", sol::property([](const Document& self) { return self.major_mode_; }),
        "path", sol::property([](const Document& self) {
            return self.path_.transform([](const std::filesystem::path& path) { return path.string(); });
        }),
        "size", sol::property([](const Document& self) { return self.data_.size(); }),

        // Static functions.
        "set_open_callback", &Document::set_open_callback,

        // Functions.
        "insert", &Document::insert,
        "remove", &Document::remove,
        "set_major_mode", [&editor](Document& self, const std::string& mode) {
            self.major_mode_ = editor.get_mode(mode);
        },
        "add_minor_mode", [&editor](Document& self, const std::string& mode) {
            self.minor_modes_.push_back(editor.get_mode(mode));
        },
        "remove_minor_mode", [](Document& self, const std::string& name) {
            std::erase_if(self.minor_modes_, [&name](const std::shared_ptr<Mode>& mode) {
                return mode->name_ == name;
            });
        },
        "toggle_minor_mode", [&editor](Document& self, const std::string& name) {
            if (std::erase_if(self.minor_modes_, [&name](const std::shared_ptr<Mode>& mode) {
                return mode->name_ == name;
            }) == 0) {
                self.minor_modes_.push_back(editor.get_mode(name));
            }
        },
        "has_minor_mode", [](const Document& self, const std::string& name) {
            return std::ranges::any_of(self.minor_modes_, [&name](const std::shared_ptr<Mode>& mode) {
                return mode->name_ == name;
            });
        });
    // clang-format on
}

void Editor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Editor>("Editor",
        // Properties.
        "active_viewport", sol::property([](const Editor& self) { return self.active_viewport_; }),
        "quit", [](Editor& self) { self.close_active_viewport(); },

        // Functions.
        "set_status_message", &Editor::set_status_message,
        "get_mode", &Editor::get_mode,
        "set_global_mode", [](Editor& self, const std::string& mode) { self.global_mode_ = self.get_mode(mode); },
        "add_global_minor_mode", [](Editor& self, const std::string& mode) {
            self.global_minor_modes_.push_back(self.get_mode(mode));
        },
        "remove_global_minor_mode", [](Editor& self, const std::string& name) {
          std::erase_if(self.global_minor_modes_, [&name](const std::shared_ptr<Mode>& mode) {
              return mode->name_ == name;
          });
        },
        "enter_mini_buffer", &Editor::enter_mini_buffer,
        "exit_mini_buffer", &Editor::exit_mini_buffer,
        "split_vertical", [](Editor& self) { self.split_active_viewport(true, 0.5); },
        "split_horizontal", [](Editor& self) { self.split_active_viewport(false, 0.5); },
        "resize_split", [](Editor& self, const float delta) { self.resize_active_viewport_split(delta); },
        "navigate_splits", &Editor::navigate_window,
        "next_key", [](Editor& self, const sol::protected_function& cmd) {
          self.input_handler_ = [cmd](Editor& editor, Key key) {
              if (cmd.valid()) {
                  if (const auto res = cmd(editor, key); !res.valid()) {
                      const sol::error err = res;
                      util::log::set_status_message(err.what());
                  }
              } else {
                  util::log::set_status_message("The next key function is not valid.");
              }
          };
        });
    // clang-format on
}

void Face::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Face>("Face",
        // Properties.
        "fg", sol::property(
            [](const Face& face) { return face.fg_; },
            [](Face& face, const std::optional<Rgb> fg) { face.fg_ = fg; }
        ),
        "bg", sol::property(
            [](const Face& face) { return face.bg_; },
            [](Face& face, const std::optional<Rgb> bg) { face.bg_ = bg; }
        ),

        // Functions.
        sol::call_constructor, sol::factories(
            [] { return Face{}; },
            [](const sol::table& table) {
                Face f{};
                if (const auto fg = table["fg"]; fg.valid() && fg.is<Rgb>()) { f.fg_ = fg.get<Rgb>(); }
                if (const auto bg = table["bg"]; bg.valid() && bg.is<Rgb>()) { f.bg_ = bg.get<Rgb>(); }
                return f;
            }
        ));
    // clang-format on
}

void Key::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Key>("Key",
        // Functions.
        "to_string", &Key::to_string,
        "normalize", [](const std::string_view str) -> std::string {
            if (Key key{0, KeyMod::NONE}; try_parse_string(str, key)) { return key.to_string(); }
            return std::string(str);
        });
    // clang-format on
}

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

void Rgb::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Rgb>("Rgb",
        // Functions.
        sol::call_constructor, sol::constructors<Rgb(uint8_t, uint8_t, uint8_t)>());
    // clang-format on
}

void Viewport::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        // Properties.
        "doc", &Viewport::doc_,
        "cursor", sol::property([](Viewport& self) { return &self.cur_; }),

        // Functions.
        "move_cursor", &Viewport::move_cursor,
        "toggle_gutter", [](Viewport& self) {
            self.gutter_ = !self.gutter_;
            self.adjust_viewport();
        },
        "set_mode_line", [](Viewport& self, const sol::protected_function& callback) { self.mode_line_renderer_ = callback; },
        "toggle_mode_line", [](Viewport& self) {
            self.mode_line_ = !self.mode_line_;
            self.adjust_viewport();
        },
        "scroll_up", [](Viewport& self, const std::size_t n = 1) { self.scroll_up(n); },
        "scroll_down", [](Viewport& self, const std::size_t n = 1) { self.scroll_down(n); },
        "scroll_left", [](Viewport& self, const std::size_t n = 1) { self.scroll_left(n); },
        "scroll_right", [](Viewport& self, const std::size_t n   = 1) { self.scroll_right(n); });
    // clang-format on
}

void Window::init_bridge(sol::table& core) {
    // clang-format off
    core.new_enum("Direction",
        "Left", Direction::LEFT,
        "Right", Direction::RIGHT,
        "Up", Direction::UP,
        "Down", Direction::DOWN);
    // clang-format on
}
