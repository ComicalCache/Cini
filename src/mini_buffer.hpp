#ifndef MODE_LINE_HPP_
#define MODE_LINE_HPP_

#include <memory>
#include <string_view>

#include <sol/sol.hpp>

struct Editor;
struct Viewport;

/// Command line container.
struct MiniBuffer {
public:
    std::shared_ptr<Viewport> viewport_;
    /// The previous active Viewport to restore focus.
    std::shared_ptr<Viewport> prev_viewport_{nullptr};

public:
    MiniBuffer(std::size_t width, std::size_t height, lua_State* L);
};

#endif
