#include "window_manager.hpp"

#include <ranges>

#include "../types/direction.hpp"
#include "../viewport.hpp"
#include "window.hpp"

auto WindowManager::find_viewport(const std::function<bool(const std::shared_ptr<Viewport>&)>& pred)
    -> std::shared_ptr<Viewport> {
    if (!this->root_) { return nullptr; }

    return this->root_->find_viewport(pred);
}

void WindowManager::set_root(std::shared_ptr<Viewport> viewport) {
    this->root_ = std::make_shared<Window>(std::move(viewport));
    this->active_viewport_ = this->root_->viewport_;

    if (this->width_ > 0 && this->height_ > 0) { this->root_->resize(0, 0, this->width_, this->height_); }
}

void WindowManager::resize(const std::size_t width, const std::size_t height) {
    this->width_ = width;
    this->height_ = height;

    if (this->root_) { this->root_->resize(0, 0, width, height); }
}

auto WindowManager::render(Display& display, const sol::protected_function& resolve_face) -> bool {
    if (!this->root_) { return false; }

    return this->root_->render(display, resolve_face);
}

void WindowManager::split(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport) {
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

void WindowManager::split_root(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport) {
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

auto WindowManager::close() -> std::shared_ptr<Viewport> {
    if (!this->root_ || !this->active_viewport_) { return nullptr; }

    if (this->root_->viewport_) { return nullptr; }

    auto [parent, child] = this->root_->find_parent(this->active_viewport_);
    const auto new_node = child == 1 ? parent->child_2_ : parent->child_1_;

    *parent = *new_node;

    if (new_node->viewport_) {
        this->active_viewport_ = new_node->viewport_;
    } else {
        this->active_viewport_ = new_node->find_viewport();
    }

    this->root_->resize(0, 0, this->width_, this->height_);

    return this->active_viewport_;
}

void WindowManager::resize_split(float delta) {
    if (!this->root_ || this->root_->viewport_ == this->active_viewport_) { return; }

    // FIXME: replace _N with _ when upgrading to C++26.
    auto [parent, _1] = this->root_->find_parent(this->active_viewport_);
    parent->ratio_ = std::clamp(parent->ratio_ + delta, 0.1F, 0.9F);
    this->root_->resize(0, 0, this->width_, this->height_);
}

void WindowManager::navigate(Direction direction) {
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
