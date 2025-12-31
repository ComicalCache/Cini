#include "document.hpp"

#include <ranges>

#include "editor.hpp"
#include "util.hpp"

void Document::init_bridge(Editor& editor, sol::table& core) {
    // clang-format off
    core.new_usertype<Document>("Document",
        "insert", &Document::insert,
        "remove", &Document::remove,
        "set_major_mode", [](Document& self, const Mode& mode) { self.major_mode_ = mode; },
        "add_minor_mode", [&editor](Document& self, const std::string& mode) {
            self.minor_modes_.push_back(editor.get_mode(mode));
        },
        "remove_minor_mode", [](Document& self, const std::string& name) {
            std::erase_if(self.minor_modes_, [&](const Mode& mode) { return mode.name_ == name; });
        },
        "toggle_minor_mode", [&editor](Document& self, const std::string& name) {
            if (std::erase_if(self.minor_modes_, [&](const Mode& mode) { return mode.name_ == name; }) == 0) {
                self.minor_modes_.push_back(editor.get_mode(name));
            }
        });
    // clang-format on
}

Document::Document(std::optional<std::filesystem::path> path)
    : path_{std::move(path)} {
    if (!this->path_) { return; }
    if (const auto res = util::read_file(*this->path_); res) { this->data_ = *res; }
}

std::string_view Document::data() const { return std::string_view(this->data_); }

std::size_t Document::line_count() const { return std::ranges::distance(this->data_ | std::views::split('\n')); }

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

void Document::remove(const std::size_t pos, const std::size_t len) {
    assert(pos + len < this->data_.size());

    this->data_.erase(pos, len);
}
