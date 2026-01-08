#include "../editor.hpp"

#include <sol/sol.hpp>

#include "../key.hpp"
#include "../mode.hpp"
#include "../viewport.hpp"
#include "../util/log.hpp"

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
                      log::set_status_message(err.what());
                  }
              } else {
                  log::set_status_message("The next key function is not valid.");
              }
          };
        });
    // clang-format on
}
