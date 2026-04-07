#include "document_view.hpp"

#include "document.hpp"
#include "editor.hpp"
#include "util/assert.hpp"

DocumentView::DocumentView(std::shared_ptr<Document> doc, sol::state& lua)
    : doc_{std::move(doc)}, properties_{lua.create_table()} {
    ASSERT(this->doc_, "DocumentView must be initialized with a valid Document");
}

auto DocumentView::move_cursor(const cursor::move_fn& move_fn, const std::size_t n) -> bool {
    auto post = this->cur_;
    move_fn(post, *this, n);
    auto target = post.point(*this);

    if (!Editor::instance()->emit_boolean_event("cursor::before-move", this->shared_from_this(), target)) {
        return false;
    }

    this->cur_ = post;

    Editor::instance()->emit_event("cursor::after-move", this->shared_from_this(), target);
    return true;
}

void DocumentView::reset_cursor() {
    this->cur_ = Cursor{
        .pos_ = Position{.row_ = 0, .col_ = 0},
          .pref_col_ = 0
    };
}

void DocumentView::add_view_property(
    const std::size_t start, const std::size_t end, const std::string& key, sol::object value) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->doc_->size(), "");

    this->view_properties_.add(start, end, key, std::move(value));
}

void DocumentView::remove_view_property(const std::size_t start, const std::size_t end, const std::string_view key) {
    ASSERT(start <= end, "");
    ASSERT(end <= this->doc_->size(), "");

    this->view_properties_.remove(start, end, key);
}

void DocumentView::clear_view_properties(const sol::optional<std::string>& key) { this->view_properties_.clear(key); }
void DocumentView::optimize_view_properties(std::string_view key) { this->view_properties_.merge(key); }

auto DocumentView::get_view_property(const std::size_t pos, const std::string_view key) const -> sol::object {
    ASSERT(pos <= this->doc_->size(), "");

    return this->view_properties_.get_property(pos, key);
}

auto DocumentView::get_view_properties(const std::size_t pos, sol::state& lua) const -> sol::table {
    ASSERT(pos <= this->doc_->size(), "");

    return this->view_properties_.get_properties(pos, lua);
}

auto DocumentView::get_all_view_properties(const std::string_view key, sol::state& lua) const -> sol::table {
    return this->view_properties_.get_all_properties(key, lua);
}

auto DocumentView::get_raw_view_property(const std::size_t pos, const std::string_view key) const -> const Property* {
    ASSERT(pos <= this->doc_->size(), "");

    return this->view_properties_.get_raw_property(pos, key);
}

auto DocumentView::clone() const -> std::shared_ptr<DocumentView> {
    // Manually create DocumentView to only emit the creation event after it has been fully cloned.
    auto view = std::make_shared<DocumentView>(this->doc_, Editor::instance()->lua_);
    view->doc_->views_.push_back(view);
    view->cur_ = this->cur_;

    for (const auto& [k, v]: this->properties_) { view->properties_[k] = v; }
    view->view_properties_ = this->view_properties_;

    Editor::instance()->document_views_.push_back(view);
    Editor::instance()->emit_event("document_view::created", view);

    return view;
}
