#ifndef ASYNC_PROCESS_HPP_
#define ASYNC_PROCESS_HPP_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <uv.h>

#include "util/ansi_text_stream.hpp"
#include "util/instance_tracker.hpp"

struct Document;

/// An abstraction of an asynchronously running process managed by libuv.
struct AsyncProcess : public InstanceTracker<AsyncProcess>, public std::enable_shared_from_this<AsyncProcess> {
public:
    std::string command_;
    std::vector<std::string> args_;

    std::shared_ptr<Document> doc_;

private:
    AnsiTextStream ansi_parser_;

    std::vector<char*> libuv_args_;

    std::vector<std::string> env_strings_{};
    std::vector<char*> libuv_env_{};

    std::optional<std::size_t> insert_pos_;

    uv_process_t process_{};
    uv_process_options_t options_{};
    uv_pipe_t stdout_{};
    uv_pipe_t stderr_{};
    std::array<uv_stdio_container_t, 3> stdio_{};

    std::size_t closed_handles_{0};

    int64_t exit_status_{0};

public:
    AsyncProcess(
        std::string command, std::vector<std::string> args, std::shared_ptr<Document> doc,
        std::optional<std::size_t> insert_pos = std::nullopt);

    AsyncProcess(const AsyncProcess&) = delete;
    auto operator=(const AsyncProcess&) -> AsyncProcess& = delete;
    AsyncProcess(AsyncProcess&&) = delete;
    auto operator=(AsyncProcess&&) -> AsyncProcess& = delete;

    auto spawn() -> bool;
    void kill();

private:
    static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    static void on_close(uv_handle_t* handle);
    static void on_exit(uv_process_t* req, int64_t status, int signal);
};

#endif
