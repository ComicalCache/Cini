#include "window.hpp"

#include "../viewport.hpp"

Window::Window(std::shared_ptr<Viewport> viewport)
    : viewport_{std::move(viewport)}, child_1_{nullptr}, child_2_{nullptr}, vertical_{false} {}

Window::Window(std::shared_ptr<Window> child_1, std::shared_ptr<Window> child_2, const bool vertical)
    : viewport_{nullptr}, child_1_{std::move(child_1)}, child_2_{std::move(child_2)}, vertical_{vertical} {}

auto Window::find_viewport(const std::function<bool(const std::shared_ptr<Viewport>&)>& pred) const
    -> std::shared_ptr<Viewport> {
    if (this->viewport_) {
        if (pred(this->viewport_)) { return this->viewport_; }

        return nullptr;
    }

    // Always prefer the first child if not leaf.
    if (auto res = this->child_1_->find_viewport(pred)) { return res; }
    return this->child_2_->find_viewport(pred);
}

void Window::resize(const std::size_t x, const std::size_t y, const std::size_t w, const std::size_t h) const {
    if (this->viewport_) {
        this->viewport_->resize(w, h, Position{.row_ = y, .col_ = x});
    } else {
        auto w1 = w;
        auto h1 = h;
        auto w2 = w;
        auto h2 = h;
        auto x2 = x;
        auto y2 = y;

        if (this->vertical_) {
            h1 = static_cast<std::size_t>(static_cast<float>(h) * this->ratio_);
            h2 = h - h1;
            y2 = y + h1;
        } else {
            w1 = static_cast<std::size_t>(static_cast<float>(w) * this->ratio_);
            w2 = w - w1;
            x2 = x + w1;
        }

        this->child_1_->resize(x, y, w1, h1);
        this->child_2_->resize(x2, y2, w2, h2);
    }
}

auto Window::render(Display& display, const sol::protected_function& resolve_face) const -> bool {
    if (this->viewport_) {
        if (!this->viewport_->render(display, resolve_face)) { return false; }
    } else {
        if (!this->child_1_->render(display, resolve_face)) { return false; }
        if (!this->child_2_->render(display, resolve_face)) { return false; }
    }

    return true;
}

auto Window::find_parent(const std::shared_ptr<Viewport>& target) -> std::pair<Window*, std::size_t> {
    if (this->viewport_) { return {nullptr, 0}; }

    if (this->child_1_->viewport_ == target) { return {this, 1}; }
    if (this->child_2_->viewport_ == target) { return {this, 2}; }

    if (auto res = this->child_1_->find_parent(target); res.first) { return res; }
    return this->child_2_->find_parent(target);
}

auto Window::get_path(const std::shared_ptr<Viewport>& target, std::vector<std::pair<Window*, std::size_t>>& path)
    -> bool {
    if (this->viewport_ == target) { return true; }
    if (this->viewport_) { return false; }

    // Search down the first child.
    path.emplace_back(this, 1);
    if (this->child_1_->get_path(target, path)) { return true; }
    path.pop_back();

    // Search down the second child.
    path.emplace_back(this, 2);
    if (this->child_2_->get_path(target, path)) { return true; }
    path.pop_back();

    return false;
}

auto Window::edge_leaf(const bool first) const -> std::shared_ptr<Viewport> {
    if (this->viewport_) { return this->viewport_; }

    return (first ? this->child_1_ : this->child_2_)->edge_leaf(first);
}
