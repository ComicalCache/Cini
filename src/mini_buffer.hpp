#ifndef MODE_LINE_HPP_
#define MODE_LINE_HPP_

#include <memory>
#include <string_view>

struct Editor;
struct Viewport;

/// Command line container.
struct MiniBuffer {
public:
    std::shared_ptr<Viewport> viewport_;
    /// The previous active Viewport to restore focus.
    std::shared_ptr<Viewport> prev_viewport_{nullptr};

public:
    MiniBuffer(std::size_t width, std::size_t height);

    /// Sets the mode of the mini buffer.
    void set_mode(std::string_view mode, Editor& editor);
};

#endif
