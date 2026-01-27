#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <memory>
#include <vector>

#include <sol/forward.hpp>

struct Display;
struct Viewport;

/// Tiling tree Window.
struct Window {
public:
    /// Viewport of the Window if it is a leaf.
    std::shared_ptr<Viewport> viewport_;

    /// Children of the Window if it is a Node.
    std::shared_ptr<Window> child_1_;
    std::shared_ptr<Window> child_2_;

    /// Size ratio of child Windows if it is a Node.
    float ratio_{0.5F};

    /// Split direction of child Windows if it is a Node.
    bool vertical_;

public:
    /// Searches a Window tree for the first Viewport it finds.
    static auto find_viewport(const std::shared_ptr<Window>& node) -> std::shared_ptr<Viewport>;

    explicit Window(std::shared_ptr<Viewport> viewport);
    Window(std::shared_ptr<Window> child_1, std::shared_ptr<Window> child_2, bool vertical);

    /// Propagates resize events through the tree and applies them on leaves.
    void resize(std::size_t x, std::size_t y, std::size_t w, std::size_t h) const;
    /// Propagates render events through the tree and applies them on leaves.
    auto render(Display& display, const sol::protected_function& resolve_face) const -> bool;

    /// Finds the parent node of a specific Viewport.
    auto find_parent(const std::shared_ptr<Viewport>& target) -> std::pair<Window*, std::size_t>;
    /// Finds the path to the target if possible.
    auto get_path(const std::shared_ptr<Viewport>& target, std::vector<std::pair<Window*, std::size_t>>& path) -> bool;
    // Finds the leaf on the specified edge of this subtree.
    [[nodiscard]]
    auto edge_leaf(bool first) const -> std::shared_ptr<Viewport>;
};

#endif
