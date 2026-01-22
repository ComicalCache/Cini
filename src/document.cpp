#include "document.hpp"

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

    this->build_line_indices();
}

auto Document::line_count() const -> std::size_t { return this->line_indices_.size(); }

auto Document::size() const -> std::size_t { return this->data_.size(); }

void Document::insert(const std::size_t pos, const std::string_view data) {
    ASSERT(pos <= this->data_.size(), "");

    this->data_.insert(pos, data);
    this->text_properties_.update_on_insert(pos, data.size());

    this->update_line_indices_on_insert(pos, data);
}

void Document::remove(const std::size_t start, const std::size_t end) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->data_.erase(start, end - start);
    this->text_properties_.update_on_remove(start, end);

    this->update_line_indices_on_remove(start, end);
}

void Document::clear() {
    this->data_.clear();
    this->text_properties_.clear(sol::nullopt);

    this->build_line_indices();
}

void Document::replace(const std::size_t start, const std::size_t end, const std::string_view new_data) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->remove(start, end);
    this->insert(start, new_data);
}

auto Document::line(std::size_t nth) const -> std::string_view {
    ASSERT(nth < this->line_count(), "");

    const auto start = this->line_indices_[nth];

    std::size_t count = std::string::npos;
    if (nth + 1 < this->line_indices_.size()) { count = this->line_indices_[nth + 1] - start; }

    return std::string_view{this->data_}.substr(start, count);
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

void Document::add_text_property(const std::size_t start, const std::size_t end, std::string key, sol::object value) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->text_properties_.add(start, end, std::move(key), std::move(value));
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

void Document::build_line_indices() {
    this->line_indices_.clear();
    this->line_indices_.emplace_back(0);

    for (std::size_t idx = 0; idx < this->data_.size(); idx += 1) {
        if (this->data_[idx] == '\n') { this->line_indices_.emplace_back(idx + 1); }
    }
}

void Document::update_line_indices_on_insert(const std::size_t pos, const std::string_view data) {
    auto start = std::ranges::upper_bound(this->line_indices_, pos);

    // Shift subsequent line starts by the inserted length.
    for (auto elem = start; elem != this->line_indices_.end(); elem++) { *elem += data.size(); }

    // Insert lines from the inserted text.
    if (data.find('\n') != std::string_view::npos) {
        std::vector<std::size_t> add;
        for (std::size_t idx = 0; idx < data.size(); idx += 1) {
            if (data[idx] == '\n') { add.push_back(pos + idx + 1); }
        }

        this->line_indices_.insert(start, add.begin(), add.end());
    }
}

void Document::update_line_indices_on_remove(const std::size_t start, const std::size_t end) {
    const auto len = end - start;

    auto it = this->line_indices_.erase(
        std::ranges::upper_bound(this->line_indices_, start), std::ranges::upper_bound(this->line_indices_, end));

    // Shift subsequent line starts by the deleted length.
    for (; it != this->line_indices_.end(); it++) { *it -= len; }
}
