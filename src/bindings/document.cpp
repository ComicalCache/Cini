#include "../document.hpp"

#include <sol/sol.hpp>

#include "../editor.hpp"
#include "../mode.hpp"

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
