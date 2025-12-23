#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <memory>

#include "cursor.hpp"
#include "display.hpp"
#include "document.hpp"

/// Viewport abstracting a Display region.
struct Viewport {
private:
    /// The backing Document that is to be rendered.
    std::shared_ptr<Document> doc_;

    bool gutter_{true};
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

    /// Resizes the viewport.
    void resize(std::size_t width, std::size_t height, Position offset);
    /// Renders the viewport to the Display.
    void render(Display& display) const;
    /// Renders the viewport's cursor to the Display.
    void render_cursor(Display& display) const;
};

#endif
