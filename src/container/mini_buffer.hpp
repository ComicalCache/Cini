#ifndef MODE_LINE_HPP_
#define MODE_LINE_HPP_

#include <memory>

#include <sol/forward.hpp>

struct ScriptEngine;
struct Viewport;

/// Command line container.
struct MiniBuffer {
public:
    std::shared_ptr<Viewport> viewport_;
    /// The previous active Viewport to restore focus.
    std::shared_ptr<Viewport> prev_viewport_{nullptr};

public:
    MiniBuffer(std::size_t width, std::size_t height, ScriptEngine& script_engine);

    void set_status_message(std::string_view message) const;
    void clear_status_message() const;
};

#endif
