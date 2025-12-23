#ifndef EDITOR_HPP_
#define EDITOR_HPP_

#include <memory>
#include <vector>

#include <uv.h>
#include <sol/sol.hpp>

#include "display.hpp"
#include "document.hpp"
#include "key.hpp"
#include "viewport.hpp"

/// State of the entire editor.
struct Editor {
private:
    /// Handle to Lua.
    std::unique_ptr<sol::state> lua_;
    /// Handle to the libuv loop.
    uv_loop_t* loop_;

    /// Stdin buffer.
    std::string input_buff_{};
    /// Stdin handle.
    uv_tty_t tty_in_{};
    /// Stdout handle.
    uv_tty_t tty_out_{};

    uv_signal_t sigwinch_{};
    uv_signal_t sigint_{};
    uv_signal_t sigquit_{};

    /// Timer on receiving a lone Esc (either user Esc or split escape sequence).
    uv_timer_t esc_timer_{};

    /// Editor Display.
    Display display_{};
    /// Editor Viewports.
    std::vector<Viewport> viewports_{};
    /// Opened Documents.
    std::vector<std::shared_ptr<Document>> documents_{};

    // TODO
    std::size_t active_viewport_{0};

public:
    Editor();
    ~Editor();

    /// Initializes libuv.
    Editor& init_uv();
    /// Initializes Lua.
    Editor& init_lua();
    /// Sets up the bridge to make structs and functions available in Lua.
    Editor& init_bridge();
    /// Initializes other editor state.
    Editor& init_state();
    /// Starts the main libuv loop.
    void run();

private:
    /// Allocates a buffer for libuv to write stdin data.
    static void alloc_input(uv_handle_t*, size_t, uv_buf_t* buf);
    /// Callback for libuv on stdin events.
    static void input(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    /// Callback on resize events.
    static void resize(uv_signal_t* handle, int);
    /// Callback on quit events.
    static void quit(uv_signal_t* handle, int);

    /// Callback on receiving a lone Esc.
    static void esc_timer(uv_timer_t* handle);

    /// Renders the editor to the display.
    void render();

    /// Processes the keypress.
    void process_key(Key key);
};

#endif
