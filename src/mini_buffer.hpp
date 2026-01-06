#ifndef MODE_LINE_HPP_
#define MODE_LINE_HPP_

#include <memory>

struct Editor;
struct Viewport;

/// Command line container.
struct MiniBuffer {
public:
    std::shared_ptr<Viewport> viewport_;
    /// The previous active Viewport.
    std::shared_ptr<Viewport> prev_viewport_{nullptr};

public:
    MiniBuffer(std::size_t width, std::size_t height);
};

#endif
