#include "window.hpp"

#include "direction.hpp"
#include "viewport.hpp"

void Window::init_bridge(sol::table& core) {
    // clang-format off
    core.new_enum("Direction",
        "Left", Direction::LEFT,
        "Right", Direction::RIGHT,
        "Up", Direction::UP,
        "Down", Direction::DOWN);
    // clang-format on
}

Window::Window(const std::shared_ptr<Viewport>& viewport)
    : viewport_{viewport}, child_1_{nullptr}, child_2_{nullptr}, vertical_{false} {}

Window::Window(std::shared_ptr<Window> child_1, std::shared_ptr<Window> child_2, const bool vertical)
    : viewport_{nullptr}, child_1_{std::move(child_1)}, child_2_{std::move(child_2)}, vertical_{vertical} {}

void Window::resize(const std::size_t x, const std::size_t y, const std::size_t w, const std::size_t h) const {
    if (this->viewport_) { // Leaf.
        this->viewport_->resize(w, h, Position{y, x});
    } else { // Node.
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

bool Window::render(Display& display, const Editor& editor) const {
    if (this->viewport_) { // Leaf.
        if (!this->viewport_->render(display, editor)) { return false; }
    } else { // Node.
        if (!this->child_1_->render(display, editor)) { return false; }
        if (!this->child_2_->render(display, editor)) { return false; }
    }

    return true;
}

std::pair<Window*, std::size_t> Window::find_parent(const std::shared_ptr<Viewport>& target) {
    if (this->viewport_) { // Leaf.
        return {nullptr, 0};
    } else { // Node.
        if (this->child_1_->viewport_ == target) { return {this, 1}; }
        if (this->child_2_->viewport_ == target) return {this, 2};

        if (auto res = this->child_1_->find_parent(target); res.first) { return res; }
        return this->child_2_->find_parent(target);
    }
}

bool Window::get_path(const std::shared_ptr<Viewport>& target, std::vector<std::pair<Window*, std::size_t>>& path) {
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

std::shared_ptr<Viewport> Window::edge_leaf(const bool first) const {
    if (this->viewport_) { // Leaf.
        return this->viewport_;
    } else { // Node.
        return (first ? this->child_1_ : this->child_2_)->edge_leaf(first);
    }
}
