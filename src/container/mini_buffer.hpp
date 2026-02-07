#ifndef MODE_LINE_HPP_
#define MODE_LINE_HPP_

#include <memory>

#include <sol/forward.hpp>

struct Viewport;

/// The Mini Buffer is the small command prompt at the bottom of the screen. This type is a convenience container for
/// managing the life-cycle of it.
struct MiniBuffer {
public:
    std::shared_ptr<Viewport> viewport_;
    /// The previous active Viewport to restore focus.
    std::weak_ptr<Viewport> prev_viewport_{};

public:
    MiniBuffer(std::size_t width, std::size_t height, sol::state& lua_);

    void set_status_message(std::string_view message, std::string_view mode) const;
    void clear_status_message() const;
};

#endif
