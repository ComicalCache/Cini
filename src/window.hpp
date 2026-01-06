#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <sol/sol.hpp>

struct Display;
struct Editor;
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
    float ratio_{0.5f};

    /// Split direction of child Windows if it is a Node.
    bool vertical_;

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    explicit Window(const std::shared_ptr<Viewport>& viewport);
    Window(std::shared_ptr<Window> child_1, std::shared_ptr<Window> child_2, bool vertical);

    /// Propagates resize events through the tree and applies them on leafs.
    void resize(std::size_t x, std::size_t y, std::size_t w, std::size_t h) const;
    /// Propagates render events through the tree and applies them on leafs.
    void render(Display& display, const Editor& editor) const;

    /// Finds the parent node of a specific Viewport.
    std::pair<Window*, std::size_t> find_parent(const std::shared_ptr<Viewport>& target);
    /// Finds the path to the target if possible.
    bool get_path(const std::shared_ptr<Viewport>& target, std::vector<std::pair<Window*, std::size_t>>& path);
    // Finds the leaf on the specified edge of this subtree.
    [[nodiscard]] std::shared_ptr<Viewport> edge_leaf(bool first) const;
};

#endif
