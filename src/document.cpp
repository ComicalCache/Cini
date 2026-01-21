#include "document.hpp"

#include <ranges>

#include "editor.hpp"
#include "regex.hpp"
#include "util/assert.hpp"
#include "util/fs.hpp"
#include "util/math.hpp"

Document::Document(std::optional<std::filesystem::path> path, lua_State* L)
    : properties_{sol::state_view{L}.create_table()}, path_{std::move(path)} {
    if (this->path_) {
        if (const auto res = fs::read_file(*this->path_); res) { // Set data on success.
            this->data_ = *res;
        } else { // Set the status message.
            // TODO: log error.
        }
    }
}

auto Document::line_count() const -> std::size_t {
    return std::max(static_cast<std::ptrdiff_t>(1), std::ranges::distance(this->data_ | std::views::split('\n')));
}

auto Document::size() const -> std::size_t { return this->data_.size(); }

void Document::insert(const std::size_t pos, const std::string_view data) {
    ASSERT(pos <= this->data_.size(), "");

    this->data_.insert(pos, data);
    this->text_properties_.update_on_insert(pos, data.size());
}

void Document::remove(const std::size_t start, const std::size_t end) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->data_.erase(start, end - start);
    this->text_properties_.update_on_remove(start, end);
}

void Document::clear() {
    this->data_.clear();
    this->text_properties_.clear(sol::nullopt);
}

void Document::replace(const std::size_t start, const std::size_t end, const std::string_view new_data) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->remove(start, end);
    this->insert(start, new_data);
}

auto Document::line(std::size_t nth) const -> std::string_view {
    ASSERT(nth < this->line_count(), "");

    auto line =
        this->data_ | std::views::chunk_by([](auto a, auto) -> auto { return a != '\n'; }) | std::views::drop(nth);
    if (line.begin() == line.end()) { return ""; }

    return {&*line.front().begin(), static_cast<std::size_t>(std::ranges::distance(line.front()))};
}

auto Document::slice(const std::size_t start, const std::size_t end) const -> std::string_view {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    return std::string_view{this->data_.data() + start, end - start};
}

auto Document::search(const std::string_view pattern) const -> std::vector<RegexMatch> {
    return Regex{pattern}.search(this->data_);
}

auto Document::search_forward(const std::string_view pattern) const -> std::vector<RegexMatch> {
    return Regex{pattern}.search(this->data_.data() + this->point_);
}

auto Document::search_backward(const std::string_view pattern) const -> std::vector<RegexMatch> {
    return Regex{pattern}.search(
        std::string_view{this->data_.data(), math::sub_sat(this->point_, static_cast<std::size_t>(1))});
}

void Document::add_text_property(
    const std::size_t start, const std::size_t end, const std::string_view key, const sol::object& value) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->text_properties_.add(start, end, key, value);
}

void Document::remove_text_property(const std::size_t start, const std::size_t end, const std::string_view key) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->text_properties_.remove(start, end, key);
}

void Document::clear_text_properties(const sol::optional<std::string_view>& key) { this->text_properties_.clear(key); }

void Document::optimize_text_properties(const std::string_view key) { this->text_properties_.merge(key); }

auto Document::get_text_property(const std::size_t pos, const std::string_view key) const -> sol::object {
    ASSERT(pos <= this->data_.size(), "");

    return this->text_properties_.get_property(pos, key);
}

auto Document::get_text_properties(const std::size_t pos, lua_State* L) const -> sol::table {
    ASSERT(pos <= this->data_.size(), "");

    return this->text_properties_.get_all_properties(pos, L);
}

auto Document::get_raw_text_property(const std::size_t pos, const std::string_view key) const -> const Property* {
    ASSERT(pos <= this->data_.size(), "");

    return this->text_properties_.get_raw_property(pos, key);
}
