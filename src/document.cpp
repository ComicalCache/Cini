#include "document.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <utility>

#include "util.hpp"

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
