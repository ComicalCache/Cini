#ifndef EDITOR_HPP_
#define EDITOR_HPP_

#include <filesystem>

#include <sol/sol.hpp>
#include <uv.h>

#include "display.hpp"
#include "mini_buffer.hpp"

enum struct Direction;
struct Document;
struct Key;
struct Viewport;
struct Window;

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
    /// Timer to clear a status message.
    uv_timer_t status_message_timer_{};

    bool is_rendering_{true};
    bool request_rendering_{false};

    /// Editor Display.
    Display display_{};
    /// Editor Viewports.
    std::shared_ptr<Window> window_{};
    /// Opened Documents.
    std::vector<std::shared_ptr<Document>> documents_{};
    /// The currently active viewport.
    std::shared_ptr<Viewport> active_viewport_{};

    MiniBuffer mini_buffer_;

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    Editor();
    ~Editor();

    /// Initializes libuv.
    Editor& init_uv();
    /// Initializes Lua.
    Editor& init_lua();
    /// Sets up the bridge to make structs and functions available in Lua.
    Editor& init_bridge();
    /// Initializes editor state.
    Editor& init_state(const std::optional<std::filesystem::path>& path);
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
    /// Callback on when to clear a status message.
    static void status_message_timer(uv_timer_t* handle);

    void enter_mini_buffer();
    void exit_mini_buffer();

    /// Splits the active Viewport. The new Viewport will be on the left or bottom.
    void split_viewport(bool vertical, float ratio);
    /// Resizes the active Viewport's split.
    void resize_viewport(float delta);
    /// Closes the active Viewport.
    void close_viewport();

    /// Navigates the Window.
    void navigate_window(Direction direction);

    /// Schedules rendering of the editor to the display.
    void render();
    /// Renders all Viewports.
    void _render();

    /// Processes the keypress.
    void process_key(Key key);
};

#endif
