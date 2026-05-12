#ifndef MOUSE_EVENT_HPP_
#define MOUSE_EVENT_HPP_

#include <cstddef>
#include <cstdint>

/// MouseEvents abstract mouse inputs on the terminal, storing the action, button, position and modifiers held during
/// it.
struct MouseEvent {
public:
    enum struct MouseAction : std::uint8_t { PRESS, RELEASE, DRAG, SCROLL_UP, SCROLL_DOWN };

    enum struct MouseButton : std::uint8_t { LEFT, MIDDLE, RIGHT, NONE };

public:
    MouseEvent::MouseAction action_;
    MouseEvent::MouseButton button_;

    /// One-indexed column.
    std::size_t x_;
    /// One-indexed row.
    std::size_t y_;
    std::size_t mod_;
};

#endif
