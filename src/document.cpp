#include "document.hpp"

#include <ranges>

#include "editor.hpp"
#include "mode.hpp"
#include "util.hpp"

sol::protected_function Document::open_callback_{};

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

void Document::set_open_callback(const sol::protected_function& open_callback) {
    Document::open_callback_ = open_callback;
}

Document::Document(std::optional<std::filesystem::path> path)
    : path_{std::move(path)} {
    if (this->path_) {
        if (const auto res = util::read_file(*this->path_); res) { // Set data on success.
            this->data_ = *res;
        } else { // Set status message.
            util::log::set_status_message(std::format("Failed to open file '{}'.", this->path_->string()));
        }
    }

    if (Document::open_callback_) {
        if (const auto res = Document::open_callback_(*this); !res.valid()) {
            const sol::error err = res;
            util::log::set_status_message(err.what());
        }
    }
}

std::string_view Document::data() const { return std::string_view(this->data_); }

std::size_t Document::line_count() const {
    if (this->data_.empty()) { return 1; }

    return std::ranges::distance(this->data_ | std::views::split('\n'));
}

std::string_view Document::line(std::size_t nth) const {
    assert(nth < this->line_count());

    auto line = this->data_ | std::views::chunk_by([](auto a, auto) { return a != '\n'; }) | std::views::drop(nth);
    if (line.begin() == line.end()) { return ""; }

    return {&*line.front().begin(), static_cast<std::size_t>(std::ranges::distance(line.front()))};
}

void Document::insert(const std::size_t pos, const std::string_view data) {
    assert(pos <= this->data_.size());

    this->data_.insert(pos, data);
}

void Document::remove(const std::size_t pos, const std::size_t n) {
    assert(pos + n <= this->data_.size());

    for (std::size_t idx = 0; idx < n; idx += 1) { this->data_.erase(pos, util::utf8::len(this->data_[pos])); }
}

void Document::clear() { this->data_.clear(); }
