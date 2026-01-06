#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <sol/sol.hpp>

#include "cursor.hpp"

struct Display;
struct Document;
struct Editor;

/// Viewport abstracting a Display region.
struct Viewport {
public:
    /// The backing Document that is to be rendered.
    std::shared_ptr<Document> doc_;

    /// Show gutter.
    bool gutter_{true};
    /// Show mode line.
    bool mode_line_{true};

private:
    std::size_t width_, height_;
    /// Offset in the Display.
    Position offset_{};
    /// Offset in the Document.
    Position scroll_{};

    /// Cursor in the Document.
    Cursor cur_{};

    /// Lua callback that provides the layout of the mode line.
    sol::function mode_line_renderer_{};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    Viewport(std::size_t width, std::size_t height, std::shared_ptr<Document> doc);

    /// Moves the cursor.
    void move_cursor(const cursor::move_fn& move_fn, std::size_t n = 1);
    /// Moves the viewport up.
    void scroll_up(std::size_t n = 1);
    /// Moves the viewport down.
    void scroll_down(std::size_t n = 1);
    /// Moves the viewport left.
    void scroll_left(std::size_t n = 1);
    /// Moves the viewport right.
    void scroll_right(std::size_t n = 1);

    /// Resizes the viewport.
    void resize(std::size_t width, std::size_t height, Position offset);
    /// Renders the viewport to the Display.
    void render(Display& display, const Editor& editor) const;
    /// Renders the viewport's cursor to the Display.
    void render_cursor(Display& display) const;

private:
    /// Adjusts the viewport to contain the cursor.
    void adjust_viewport();

    /// Renders the mode line.
    void render_mode_line(Display& display, const Editor& editor) const;

    /// Generates a syntax overlay list.
    [[nodiscard]] std::vector<const std::string*> generated_syntax_overlay(
        const Editor& editor, std::string_view line) const;
};

#endif
