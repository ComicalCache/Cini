#include "workspace.hpp"

#include <memory>
#include <ranges>

// Include required because editor.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.
#include "../editor.hpp"
#include "../types/direction.hpp"
#include "../viewport.hpp"
#include "window.hpp"

Workspace::Workspace(sol::state& lua) : mini_buffer_{0, 0, lua} {}

auto Workspace::find_viewport(const std::function<bool(const std::shared_ptr<Viewport>&)>& pred) const
    -> std::shared_ptr<Viewport> {
    if (!this->root_) { return nullptr; }

    return this->root_->find_viewport(pred);
}

void Workspace::set_root(std::shared_ptr<Viewport> viewport) {
    this->switch_viewport([&] -> std::pair<bool, bool> {
        this->root_ = std::make_shared<Window>(std::move(viewport));
        this->active_viewport_ = this->root_->viewport_;

        if (this->width_ > 0 && this->height_ > 0) { this->root_->resize(0, 0, this->width_, this->height_); }
        return {true, false};
    });
}

void Workspace::resize(const std::size_t width, const std::size_t height) {
    this->width_ = width;
    this->height_ = height;

    if (this->root_) { this->root_->resize(0, 0, width, height); }
}

auto Workspace::render(Display& display, const sol::protected_function& resolve_face) const -> bool {
    if (!this->root_) { return false; }

    return this->root_->render(display, resolve_face);
}

void Workspace::enter_mini_buffer(uv_timer_t& timer) {
    if (this->is_mini_buffer_) { return; }

    this->switch_viewport([&] -> std::pair<bool, bool> {
        uv_timer_stop(&timer);

        this->mini_buffer_.clear_status_message();

        this->is_mini_buffer_ = true;
        this->mini_buffer_.prev_viewport_ = this->active_viewport_;

        return {false, false};
    });
}

void Workspace::exit_mini_buffer() {
    if (!this->is_mini_buffer_) { return; }

    this->switch_viewport([&] -> std::pair<bool, bool> {
        this->is_mini_buffer_ = false;

        if (auto prev = this->mini_buffer_.prev_viewport_.lock(); prev) {
            this->active_viewport_ = prev;
        } else {
            this->active_viewport_ = this->find_viewport([](const auto&) -> bool { return true; });
        }

        this->mini_buffer_.prev_viewport_.reset();

        return {false, false};
    });
}

void Workspace::split(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport) {
    if (this->is_mini_buffer_) { return; }

    this->switch_viewport([&] -> std::pair<bool, bool> {
        this->_split(vertical, ratio, std::move(new_viewport));
        return {false, false};
    });
}

void Workspace::split_root(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport) {
    if (!this->root_) {
        this->set_root(new_viewport);
        return;
    }

    if ((vertical && this->active_viewport_->height_ < 8) || (!vertical && this->active_viewport_->width_ < 25)) {
        return;
    }

    auto new_leaf = std::make_shared<Window>(std::move(new_viewport));

    this->root_ = std::make_shared<Window>(this->root_, new_leaf, vertical);
    this->root_->ratio_ = ratio;
    this->active_viewport_ = new_leaf->viewport_;

    this->root_->resize(0, 0, this->width_, this->height_);
}

void Workspace::resize_split(float delta) const {
    if (this->is_mini_buffer_ || !this->root_ || this->root_->viewport_ == this->active_viewport_) { return; }

    // FIXME: replace _N with _ when upgrading to C++26.
    auto [parent, _1] = this->root_->find_parent(this->active_viewport_);
    parent->ratio_ = std::clamp(parent->ratio_ + delta, 0.1F, 0.9F);
    this->root_->resize(0, 0, this->width_, this->height_);
}

void Workspace::navigate_split(const Direction direction) {
    if (this->is_mini_buffer_) { return; }

    this->switch_viewport([&] -> std::pair<bool, bool> {
        this->_navigate_split(direction);
        return {false, false};
    });
}

auto Workspace::close_split() -> std::optional<std::shared_ptr<Viewport>> {
    if (this->is_mini_buffer_) { return std::nullopt; }

    const auto prev = this->active_viewport_;
    std::shared_ptr<Viewport> curr{};

    this->switch_viewport([&] -> std::pair<bool, bool> {
        auto doc_use_count = 0UZ;
        this->find_viewport([&](const std::shared_ptr<Viewport>& vp) -> bool {
            if (vp->doc_ == prev->doc_) { doc_use_count++; }
            return false;
        });

        curr = this->_close_split();

        return {false, doc_use_count == 0};
    });

    Editor::instance()->emit_event("viewport::destroyed", prev);

    return curr;
}

void Workspace::_split(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport) {
    if (!this->root_ || !this->active_viewport_) { return; }

    if ((vertical && this->active_viewport_->height_ < 8) || (!vertical && this->active_viewport_->width_ < 25)) {
        return;
    }

    auto new_leaf = std::make_shared<Window>(std::move(new_viewport));

    // No Split exists yet.
    if (this->root_->viewport_ == this->active_viewport_) {
        this->root_ = std::make_shared<Window>(this->root_, new_leaf, vertical);
        this->root_->ratio_ = ratio;
        this->active_viewport_ = new_leaf->viewport_;

        // Calculate dimensions.
        this->root_->resize(0, 0, this->width_, this->height_);

        return;
    }

    auto [parent, child] = this->root_->find_parent(this->active_viewport_);
    auto old_leaf = child == 1 ? parent->child_1_ : parent->child_2_;
    auto new_split = std::make_shared<Window>(old_leaf, new_leaf, vertical);
    new_split->ratio_ = ratio;

    if (child == 1) {
        parent->child_1_ = new_split;
    } else {
        parent->child_2_ = new_split;
    }

    this->active_viewport_ = new_leaf->viewport_;

    this->root_->resize(0, 0, this->width_, this->height_);
}

void Workspace::_navigate_split(Direction direction) {
    std::vector<std::pair<Window*, std::size_t>> path;
    if (!this->root_->get_path(this->active_viewport_, path)) { return; }

    for (auto& [window, child]: std::ranges::reverse_view(path)) {
        auto can_move{false};
        auto idx{0UZ};
        const auto is_vert = window->vertical_;

        switch (direction) {
            case Direction::LEFT:
                if (!is_vert && child == 2) {
                    can_move = true;
                    idx = 1;
                }
                break;
            case Direction::RIGHT:
                if (!is_vert && child == 1) {
                    can_move = true;
                    idx = 2;
                }
                break;
            case Direction::UP:
                if (is_vert && child == 2) {
                    can_move = true;
                    idx = 1;
                }
                break;
            case Direction::DOWN:
                if (is_vert && child == 1) {
                    can_move = true;
                    idx = 2;
                }
                break;
            default: std::unreachable();
        }

        if (can_move) {
            const auto sibling = idx == 1 ? window->child_1_ : window->child_2_;
            const auto prefer_first = direction == Direction::RIGHT || direction == Direction::DOWN;

            this->active_viewport_ = sibling->edge_leaf(prefer_first);
            return;
        }
    }
}

auto Workspace::_close_split() -> std::shared_ptr<Viewport> {
    if (!this->root_ || !this->active_viewport_) { return nullptr; }

    if (this->root_->viewport_) { return nullptr; }

    auto [parent, child] = this->root_->find_parent(this->active_viewport_);
    const auto new_node = child == 1 ? parent->child_2_ : parent->child_1_;

    *parent = *new_node;

    if (new_node->viewport_) {
        this->active_viewport_ = new_node->viewport_;
    } else {
        this->active_viewport_ = new_node->find_viewport([](const std::shared_ptr<Viewport>&) -> bool { return true; });
    }

    this->root_->resize(0, 0, this->width_, this->height_);

    return this->active_viewport_;
}

void Workspace::switch_viewport(std::function<std::pair<bool, bool>()>&& f) {
    const auto prev = this->is_mini_buffer_ ? this->mini_buffer_.viewport_ : this->active_viewport_;

    // Viewport switching.
    const auto [next_doc_loaded, prev_doc_unloaded] = f();

    const auto next = this->is_mini_buffer_ ? this->mini_buffer_.viewport_ : this->active_viewport_;
    if (prev != next) {
        auto prev_doc = prev ? prev->doc_ : nullptr;
        auto next_doc = next ? next->doc_ : nullptr;

        auto editor = Editor::instance();

        if (prev) { editor->emit_event("viewport::unfocus", prev); }
        if (prev_doc != next_doc) {
            if (prev_doc) { editor->emit_event("document::unfocus", prev_doc); }
            if (prev_doc_unloaded) { editor->emit_event("document::unloaded", prev_doc); }
            if (next_doc_loaded) { editor->emit_event("document::loaded", next_doc); }
            if (next_doc) { editor->emit_event("document::focus", next_doc); }
        }
        if (next) { editor->emit_event("viewport::focus", next); }
    }
}
