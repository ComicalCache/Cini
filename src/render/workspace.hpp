#ifndef WINDOW_MANAGER_HPP_
#define WINDOW_MANAGER_HPP_

#include <functional>
#include <memory>

#include <sol/forward.hpp>
#include <uv.h>

#include "../container/mini_buffer.hpp"

enum struct Direction : std::uint8_t;
struct Display;
struct Viewport;
struct Window;

/// A Workspace manages a tiling tree *including* a MiniBuffer.
///
/// Workspace data must be managed through the API of this class and never directly manipulated. Failure to do so can
/// result in UB and crashes.
struct Workspace {
public:
    bool is_mini_buffer_{false};
    MiniBuffer mini_buffer_;

    std::shared_ptr<Window> root_{nullptr};
    std::shared_ptr<Viewport> active_viewport_{nullptr};

    std::size_t width_{0};
    std::size_t height_{0};

public:
    Workspace(sol::state& lua);

    /// Searches the Window tree for the first Viewport matching a predicate.
    auto find_viewport(const std::function<bool(const std::shared_ptr<Viewport>&)>& pred) const
        -> std::shared_ptr<Viewport>;

    /// Sets the root node. The Viewport will be in Workspace::active_viewport_.
    void set_root(std::shared_ptr<Viewport> viewport);

    /// Propagates resize events through the tree and applies them on leaves.
    void resize(std::size_t width, std::size_t height);
    /// Propagates render events through the tree and applies them on leaves.
    auto render(Display& display, const sol::protected_function& resolve_face) const -> bool;

    void enter_mini_buffer(uv_timer_t& timer);
    void exit_mini_buffer();

    /// Splits the current Viewport into two. The new Viewport will be in Window::child_2_.
    void split(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport);
    // Splits the root into two. The new Viewport will be in Window::child_2_.
    void split_root(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport);
    /// Resizes the current split.
    void resize_split(float delta) const;
    /// Navigates the splits.
    void navigate_split(Direction direction);
    /// Closes the current Viewport.
    auto close_split() -> std::optional<std::shared_ptr<Viewport>>;

private:
    void _split(bool vertical, float ratio, std::shared_ptr<Viewport> new_viewport);
    void _navigate_split(Direction direction);
    auto _close_split() -> std::shared_ptr<Viewport>;

    void switch_viewport(std::function<std::pair<bool, bool>()>&& f);
};

#endif
