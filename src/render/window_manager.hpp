#ifndef WINDOW_MANAGER_HPP_
#define WINDOW_MANAGER_HPP_

#include <functional>
#include <memory>

#include <sol/forward.hpp>

enum struct Direction : std::uint8_t;
struct Display;
struct Viewport;
struct Window;

/// Manages the tiling tree.
struct WindowManager {
public:
    std::shared_ptr<Viewport> active_viewport_{nullptr};

    std::size_t width_{0};
    std::size_t height_{0};

private:
    std::shared_ptr<Window> root_{nullptr};

public:
    /// Searches the Window tree for the first Viewport matching a predicate.
    auto find_viewport(const std::function<bool(const std::shared_ptr<Viewport>&)>& pred) -> std::shared_ptr<Viewport>;

    /// Sets the root node. The Viewport will be in WindowManager::active_viewport_.
    void set_root(std::shared_ptr<Viewport> viewport);

    /// Propagates resize events through the tree and applies them on leaves.
    void resize(std::size_t width, std::size_t height);
    /// Propagates render events through the tree and applies them on leaves.
    auto render(Display& display, const sol::protected_function& resolve_face) -> bool;

    /// Splits the current Viewport into two. The new Viewport will be in Window::child_2_.
    void split(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport);
    // Splits the root into two. The new Viewport will be in Window::child_2_.
    void split_root(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport);

    /// Resizes the current split.
    void resize_split(float delta);
    /// Closes the current Viewport.
    auto close() -> std::shared_ptr<Viewport>;

    /// Navigates between splits.
    void navigate(Direction direction);
};

#endif
