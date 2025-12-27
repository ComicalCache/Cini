#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <memory>
#include <unordered_map>

#include "cursor.hpp"
#include "display.hpp"
#include "document.hpp"
#include "string_hash.hpp"

/// Viewport abstracting a Display region.
struct Viewport {
public:
    /// The backing Document that is to be rendered.
    std::shared_ptr<Document> doc_;
    /// Show gutter.
    bool gutter_{true};

private:
    /// Character replacements during rendering.
    std::unordered_map<std::string, Cell, StringHash, std::equal_to<>> replacements_;

    /// Mode line contents.
    std::string mode_line_{};

    std::size_t width_, height_;
    /// Offset in the Display.
    Position offset_{};
    /// Offset in the Document.
    Position scroll_{};

    /// Cursor in the Document.
    Cursor cur_{};

public:
    Viewport(std::size_t width, std::size_t height, std::shared_ptr<Document> doc);

    /// Moves the cursor.
    void move_cursor(cursor::move_fn move_fn, std::size_t n = 1);
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
    void render(Display& display) const;
    /// Renders the viewport's cursor to the Display.
    void render_cursor(Display& display) const;

private:
    void adjust_viewport();
};

#endif
