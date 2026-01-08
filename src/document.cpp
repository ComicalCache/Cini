#include "document.hpp"

#include <ranges>

#include "editor.hpp"
#include "mode.hpp"
#include "util.hpp"

sol::protected_function Document::open_callback_{};

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
