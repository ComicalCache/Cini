#ifndef EDITOR_HPP_
#define EDITOR_HPP_

#include <filesystem>

#include <sol/state.hpp>
#include <uv.h>

#include "container/mini_buffer.hpp"
#include "render/display.hpp"
#include "render/workspace.hpp"
#include "util/assert.hpp"

struct Document;
struct Key;
struct Viewport;

/// State of the entire editor.
struct Editor {
public:
    sol::state lua_{};

    Workspace workspace_;

private:
    struct EditorKey {};

    bool initialized_{false};

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

    Display display_{};
    /// Opened Documents.
    std::vector<std::shared_ptr<Document>> documents_{};

public:
    /// Initializes the Editor singleton.
    static void setup(const std::optional<std::filesystem::path>& path);
    /// Runs the event loop.
    static void run();
    /// Frees all resources.
    static void destroy();

    /// Returns the singleton instance of Editor.
    static auto instance() -> std::shared_ptr<Editor>;

    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    Editor(EditorKey key);
    ~Editor();

    Editor(const Editor&) = delete;
    auto operator=(const Editor&) -> Editor& = delete;
    Editor(Editor&&) = delete;
    auto operator=(Editor&&) -> Editor& = delete;

    auto create_document(std::optional<std::filesystem::path> path) -> std::shared_ptr<Document>;
    auto create_viewport(std::size_t width, std::size_t height, std::shared_ptr<Document> doc)
        -> std::shared_ptr<Viewport>;
    auto create_viewport(const std::shared_ptr<Viewport>& viewport) -> std::shared_ptr<Viewport>;

    void set_status_message(std::string_view message, bool force_viewport = false);

    /// Emits an event triggering Lua hooks listening for it.
    template<typename... Args>
    void emit_event(const std::string_view event, Args&&... args) {
        sol::protected_function run = this->lua_["Core"]["Hooks"]["run"];
        ASSERT(run.valid(), "");

        const sol::protected_function_result result = run(event, std::forward<Args>(args)...);
        if (!result.valid()) {
            this->set_status_message(
                std::format("Failed to emit event '{}':\n{}", event, static_cast<sol::error>(result).what()));
        }
    }

private:
    /// Allocates a buffer for libuv to write stdin data.
    static void alloc_input(uv_handle_t* handle, std::size_t recommendation, uv_buf_t* buf);

    /// Callback for libuv on stdin events.
    static void input(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    /// Callback on resize events.
    static void resize(uv_signal_t* handle, int code);
    /// Callback on quit events.
    static void quit(uv_signal_t* handle, int code);
    /// Callback on receiving a lone Esc.
    static void esc_timer(uv_timer_t* handle);
    /// Callback on when to clear a status message.
    static void status_message_timer(uv_timer_t* handle);

    /// Initializes libuv.
    auto init_uv() -> Editor&;
    /// Initializes the Lua runtime.
    auto init_lua() -> Editor&;
    /// Sets up the bridge to make structs and functions available in Lua.
    auto init_bridge() -> Editor&;
    /// Initializes editor state.
    auto init_state(const std::optional<std::filesystem::path>& path) -> Editor&;
    /// Frees all resources.
    void shutdown();

    /// Processes the keypress.
    void process_key(Key key);

    /// Schedules rendering of the editor to the display.
    void render();
    /// Renders all Viewports.
    void _render();
};

#endif
