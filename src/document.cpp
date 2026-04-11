#include "document.hpp"

#include <ranges>

#include <sol/state_view.hpp>

#include "document_view.hpp"
#include "editor.hpp"
#include "regex.hpp"
#include "types/operation.hpp"
#include "types/position.hpp"
#include "util/assert.hpp"
#include "util/fs.hpp"
#include "util/math.hpp"

Document::Document(std::optional<std::filesystem::path> path, sol::state& lua)
    : path_{std::move(path)}, properties_{lua.create_table()} {
    this->build_line_indices();
}

auto Document::views() -> std::vector<std::shared_ptr<DocumentView>> {
    std::vector<std::shared_ptr<DocumentView>> views;

    for (auto it = this->views_.begin(); it != this->views_.end();) {
        if (auto view = it->lock()) {
            views.push_back(view);
            ++it;
        } else {
            it = this->views_.erase(it);
        }
    }
    return views;
}

void Document::save(std::optional<std::filesystem::path> path) {
    auto editor = Editor::instance();

    if (!path && !this->path_) {
        editor->set_status_message("Please specify a filename.", "info_message");
        return;
    }

    editor->emit_event("document::before-save", this->shared_from_this());

    if (path) {
        if (!fs::write_file(*path, this->data_, std::ios::out | std::ios::trunc)) {
            editor->set_status_message("Failed to write file.", "error_message");
            return;
        }

        this->path_ = std::move(path);
        goto EXIT;
    }

    if (!fs::write_file(
            *this->path_, this->data_, std::ios::out | std::ios::trunc)) { // NOLINT(bugprone-unchecked-optional-access)
        editor->set_status_message("Failed to write file.", "error_message");
        return;
    }

EXIT:
    this->modified_ = false;
    editor->emit_event("document::after-save", this->shared_from_this());
}

auto Document::line_count() const -> std::size_t { return this->line_indices_.size(); }

auto Document::size() const -> std::size_t { return this->data_.size(); }

void Document::insert(const std::size_t pos, const std::string_view data) {
    ASSERT(pos <= this->data_.size(), "");

    auto editor = Editor::instance();
    editor->emit_event("document::before-insert", this->shared_from_this(), pos, data.size());

    if (this->recording_transaction_ && !this->applying_transaction_) {
        this->active_transaction_.operations_.emplace_back(Operation::Type::INSERT, pos, std::string(data));
    }

    this->data_.insert(pos, data);
    this->text_properties_.update_on_insert(pos, data.size());
    this->modified_ = true;

    this->update_line_indices_on_insert(pos, data);

    editor->emit_event("document::after-insert", this->shared_from_this(), pos, data.size());
}

void Document::remove(const std::size_t start, const std::size_t end) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    auto editor = Editor::instance();
    editor->emit_event("document::before-remove", this->shared_from_this(), start, end - start);

    if (this->recording_transaction_ && !this->applying_transaction_) {
        this->active_transaction_.operations_.emplace_back(
            Operation::Type::REMOVE, start, std::string(this->slice(start, end)));
    }

    this->data_.erase(start, end - start);
    this->text_properties_.update_on_remove(start, end);
    this->modified_ = true;

    this->update_line_indices_on_remove(start, end);

    editor->emit_event("document::after-remove", this->shared_from_this(), start, end - start);
}

void Document::clear() {
    auto editor = Editor::instance();
    editor->emit_event("document::before-clear", this->shared_from_this());

    this->data_.clear();
    this->text_properties_.clear(sol::nullopt);
    this->modified_ = true;

    this->build_line_indices();

    editor->emit_event("document::after-clear", this->shared_from_this());
}

void Document::replace(const std::size_t start, const std::size_t end, const std::string_view new_data) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->remove(start, end);
    this->insert(start, new_data);
    this->modified_ = true;
}

auto Document::line(std::size_t nth) const -> std::string_view {
    ASSERT(nth < this->line_count(), "");

    const auto start = this->line_indices_[nth];

    auto count = std::string::npos;
    if (nth + 1 < this->line_indices_.size()) { count = this->line_indices_[nth + 1] - start; }

    return std::string_view{this->data_}.substr(start, count);
}

auto Document::slice(const std::size_t start, const std::size_t end) const -> std::string_view {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    return std::string_view{this->data_.data() + start, end - start};
}

auto Document::line_begin_byte(const std::size_t nth) const -> std::size_t {
    ASSERT(nth < this->line_count(), "");
    return this->line_indices_[nth];
}

auto Document::line_end_byte(const std::size_t nth) const -> std::size_t {
    ASSERT(nth < this->line_count(), "");

    if (nth + 1 < this->line_indices_.size()) { return this->line_indices_[nth + 1]; }
    return this->data_.size();
}

auto Document::position_from_byte(const std::size_t byte) const -> Position {
    auto row = std::distance(this->line_indices_.begin(), std::ranges::upper_bound(this->line_indices_, byte) - 1);
    auto col = byte - this->line_indices_[row];
    return Position{.row_ = static_cast<std::size_t>(row), .col_ = col};
}

auto Document::search(const Regex& regex, const std::size_t start, const std::size_t end) const
    -> std::vector<RegexMatch> {
    ASSERT(start < this->data_.length(), "");

    auto matches = regex.search(
        std::string_view{this->data_.data() + start, math::sub_sat(std::min(this->data_.length(), end), start)});

    // Adjust the matches to have correct byte offsets, since the match indices are relative to the (shifted) input.
    for (auto& match: matches) {
        match.start_ += start;
        match.end_ += start;
    }

    return matches;
}

void Document::begin_transaction(std::size_t point) {
    if (this->recording_transaction_) { return; }

    this->recording_transaction_ = true;
    this->active_transaction_.operations_.clear();
    this->active_transaction_.point_before_ = point;
}

void Document::end_transaction(std::size_t point) {
    if (!this->recording_transaction_) { return; }

    this->recording_transaction_ = false;
    if (this->active_transaction_.operations_.empty()) { return; }

    this->active_transaction_.point_after_ = point;
    this->undo_stack_.push_back(std::move(this->active_transaction_));
    this->redo_stack_.clear();
    this->active_transaction_ = {};
}

auto Document::undo() -> std::optional<std::size_t> {
    if (this->undo_stack_.empty()) { return std::nullopt; }

    this->applying_transaction_ = true;
    auto group = std::move(this->undo_stack_.back());
    this->undo_stack_.pop_back();

    for (const auto& operation: std::views::reverse(group.operations_)) {
        if (operation.type_ == Operation::Type::INSERT) {
            this->remove(operation.pos_, operation.pos_ + operation.data_.size());
        } else {
            this->insert(operation.pos_, operation.data_);
        }
    }

    auto point = group.point_before_;
    this->redo_stack_.push_back(std::move(group));
    this->applying_transaction_ = false;

    return point;
}

auto Document::redo() -> std::optional<std::size_t> {
    if (this->redo_stack_.empty()) { return std::nullopt; }

    this->applying_transaction_ = true;
    auto group = std::move(this->redo_stack_.back());
    this->redo_stack_.pop_back();

    for (const auto& op: group.operations_) {
        if (op.type_ == Operation::Type::INSERT) {
            this->insert(op.pos_, op.data_);
        } else {
            this->remove(op.pos_, op.pos_ + op.data_.size());
        }
    }

    auto point = group.point_after_;
    this->undo_stack_.push_back(std::move(group));
    this->applying_transaction_ = false;

    return point;
}

void Document::add_text_property(
    const std::size_t start, const std::size_t end, const std::string& key, sol::object value) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->text_properties_.add(start, end, key, std::move(value));
}

void Document::remove_text_property(const std::size_t start, const std::size_t end, const std::string_view key) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->data_.size(), "");

    this->text_properties_.remove(start, end, key);
}

void Document::clear_text_properties(const sol::optional<std::string>& key) { this->text_properties_.clear(key); }
void Document::optimize_text_properties(const std::string_view key) { this->text_properties_.merge(key); }

auto Document::get_text_property(const std::size_t pos, const std::string_view key) const -> sol::object {
    ASSERT(pos <= this->data_.size(), "");

    return this->text_properties_.get_property(pos, key);
}

auto Document::get_text_properties(const std::size_t pos, sol::state& lua) const -> sol::table {
    ASSERT(pos <= this->data_.size(), "");

    return this->text_properties_.get_properties(pos, lua);
}

auto Document::get_all_text_properties(const std::string_view key, sol::state& lua) const -> sol::table {
    return this->text_properties_.get_all_properties(key, lua);
}

auto Document::get_raw_text_property(const std::size_t pos, const std::string_view key) const -> const Property* {
    ASSERT(pos <= this->data_.size(), "");

    return this->text_properties_.get_raw_property(pos, key);
}

void Document::build_line_indices() {
    this->line_indices_.clear();
    this->line_indices_.emplace_back(0);

    for (auto idx{0UZ}; idx < this->data_.size(); idx += 1) {
        if (this->data_[idx] == '\n') { this->line_indices_.emplace_back(idx + 1); }
    }
}

void Document::update_line_indices_on_insert(const std::size_t pos, const std::string_view data) {
    auto start = std::ranges::upper_bound(this->line_indices_, pos);

    // Shift subsequent line starts by the inserted length.
    for (auto elem = start; elem != this->line_indices_.end(); elem++) { *elem += data.size(); }

    // Insert lines from the inserted text.
    if (data.contains('\n')) {
        std::vector<std::size_t> add;
        for (auto idx{0UZ}; idx < data.size(); idx += 1) {
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
