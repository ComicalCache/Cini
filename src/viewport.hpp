#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <sol/protected_function.hpp>

#include "types/position.hpp"
#include "util/ansi.hpp"
#include "util/instance_tracker.hpp"

struct Display;
struct DocumentView;
struct Face;
struct MiniBuffer;
struct ViewportBinding;

/// Viewports abstract Display regions occupied by a Window. They draw a Document through a DocumentView and optional
/// gutter and mode line.
///
/// Viewport data must be managed through the API of this class and never directly inserted. Failure to do so can result
/// in UB and crashes.
struct Viewport : public InstanceTracker<Viewport>, public std::enable_shared_from_this<Viewport> {
    friend MiniBuffer;
    friend ViewportBinding;

public:
    std::shared_ptr<DocumentView> view_;

    std::size_t width_;
    std::size_t height_;

private:
    /// Show gutter.
    bool gutter_{true};
    /// Show mode line.
    mutable bool mode_line_{true};

    /// Offset in the Display.
    Position offset_{};
    /// Scrolling offset in the Document.
    Position scroll_{};

    /// Visual cursor position.
    mutable std::optional<Position> visual_cur_{};

    /// Lua callback that provides the layout of the mode line.
    sol::protected_function mode_line_callback_{};

public:
    Viewport(std::size_t width, std::size_t height, std::shared_ptr<DocumentView> view);

    /// Changes the displayed document.
    void change_document_view(const std::shared_ptr<DocumentView>& view);

    /// Moves the viewport up.
    void scroll_up(std::size_t n);
    /// Moves the viewport down.
    void scroll_down(std::size_t n);
    /// Moves the viewport left.
    void scroll_left(std::size_t n);
    /// Moves the viewport right.
    void scroll_right(std::size_t n);

    /// Adjusts the viewport to contain the cursor.
    void adjust_viewport();

    /// Resizes the viewport.
    void resize(std::size_t width, std::size_t height, Position offset);
    /// Renders the viewport to the Display, returning if the rendering was successful.
    [[nodiscard]]
    auto render(Display& display, const sol::protected_function& resolve_face) const -> bool;
    /// Renders the mode line, returning if the rendering was successful.
    [[nodiscard]]
    auto render_mode_line(Display& display, const sol::protected_function& resolve_face) const -> bool;
    /// Renders the viewport's cursor to the Display.
    void render_cursor(Display& display, ansi::CursorStyle style) const;

private:
    void _draw_gutter(
        Display& display, Face face, std::size_t gutter_width, std::optional<std::size_t> line, std::size_t y) const;
    void _draw_char(
        Display& display, Face face, std::size_t gutter_width, std::size_t content_width, std::string_view ch,
        std::size_t width, bool tab, std::size_t x, std::size_t y) const;
};

#endif
