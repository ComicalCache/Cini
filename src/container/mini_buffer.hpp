#ifndef MODE_LINE_HPP_
#define MODE_LINE_HPP_

#include <memory>

#include <sol/state.hpp>

struct Viewport;

/// Command line container.
struct MiniBuffer {
public:
    std::shared_ptr<Viewport> viewport_;
    /// The previous active Viewport to restore focus.
    std::weak_ptr<Viewport> prev_viewport_{};

public:
    MiniBuffer(std::size_t width, std::size_t height, sol::state& lua_);

    void set_status_message(std::string_view message) const;
    void clear_status_message() const;
};

#endif
