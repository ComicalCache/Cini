#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <sol/protected_function.hpp>

#include "cursor.hpp"

struct Display;
struct Document;
struct Face;
struct Window;

/// Viewport abstracting a Display region.
struct Viewport {
public:
    /// The backing Document that is to be rendered.
    std::shared_ptr<Document> doc_;

    /// Show gutter.
    bool gutter_{true};
    /// Show mode line.
    bool mode_line_{true};

    std::size_t width_;
    std::size_t height_;

private:
    /// Offset in the Display.
    Position offset_{};
    /// Scrolling offset in the Document.
    Position scroll_{};

    /// Cursor in the Document.
    Cursor cur_{};
    /// Visual cursor position.
    std::optional<Position> visual_cur_{};

    /// Lua callback that provides the layout of the mode line.
    sol::protected_function mode_line_callback_{};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    /// Searches a Window tree for the first Viewport it finds.
    static auto find_viewport(const std::shared_ptr<Window>& node) -> std::shared_ptr<Viewport>;

    Viewport(std::size_t width, std::size_t height, std::shared_ptr<Document> doc);

    /// Moves the cursor.
    void move_cursor(const cursor::move_fn& move_fn, std::size_t n);
    /// Moves the viewport up.
    void scroll_up(std::size_t n);
    /// Moves the viewport down.
    void scroll_down(std::size_t n);
    /// Moves the viewport left.
    void scroll_left(std::size_t n);
    /// Moves the viewport right.
    void scroll_right(std::size_t n);

    /// Resizes the viewport.
    void resize(std::size_t width, std::size_t height, Position offset);
    /// Renders the viewport to the Display.
    auto render(Display& display, const sol::protected_function& resolve_face) -> bool;
    /// Renders the mode line.
    auto render_mode_line(Display& display, const sol::protected_function& resolve_face) -> bool;
    /// Renders the viewport's cursor to the Display.
    void render_cursor(Display& display) const;

private:
    /// Adjusts the viewport to contain the cursor.
    void adjust_viewport();

    void _draw_gutter(
        Display& display, Face face, std::size_t gutter_width, std::optional<std::size_t> line, std::size_t y) const;
    void _draw_char(
        Display& display, Face face, std::size_t gutter_width, std::size_t content_width, std::string_view ch,
        std::size_t width, bool tab, std::size_t x, std::size_t y) const;
};

#endif
