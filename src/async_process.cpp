#include "async_process.hpp"

#include <cstring>
#include <string>

#include "document.hpp"
#include "editor.hpp"
#include "util/ansi_text_stream.hpp"
#include "util/assert.hpp"

// This only works on UNIX systems.
extern char** environ;

AsyncProcess::AsyncProcess(
    std::string command, std::vector<std::string> args, std::shared_ptr<Document> doc,
    const std::optional<std::size_t> insert_pos)
    : command_{std::move(command)}, args_{std::move(args)}, doc_{std::move(doc)}, ansi_parser_{doc_},
      insert_pos_{insert_pos} {
    ASSERT(!this->command_.empty(), "");
    ASSERT(this->doc_, "");

    this->libuv_args_.push_back(command_.data());
    for (auto& arg: args_) { this->libuv_args_.push_back(arg.data()); }
    this->libuv_args_.push_back(nullptr);
}

auto AsyncProcess::spawn() -> bool {
    auto editor{Editor::instance()};

    uv_pipe_init(editor->loop_, &this->stdout_, 0);
    uv_pipe_init(editor->loop_, &this->stderr_, 0);

    if (environ != nullptr) {
        for (auto** env{environ}; *env != nullptr; env += 1) { this->env_strings_.emplace_back(*env); }
    }

    // Enable colors.
    this->env_strings_.emplace_back("FORCE_COLOR=1");
    this->env_strings_.emplace_back("CLICOLOR_FORCE=1");
    this->env_strings_.emplace_back("TERM=xterm-256color");

    for (auto& env: this->env_strings_) { this->libuv_env_.push_back(env.data()); }
    this->libuv_env_.push_back(nullptr);

    this->options_.env = this->libuv_env_.data();

    this->options_.exit_cb = AsyncProcess::on_exit;
    this->options_.file = this->command_.c_str();
    this->options_.args = this->libuv_args_.data();

    this->options_.stdio_count = 3;
    this->options_.stdio = this->stdio_.data();

    // Enum used as flag causes this false positive.
    // NOLINTBEGIN(clang-analyzer-optin.core.EnumCastOutOfRange)

    // Ignore stdin.
    this->stdio_[0].flags = UV_IGNORE;
    this->stdio_[1].flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    this->stdio_[1].data.stream = reinterpret_cast<uv_stream_t*>(&this->stdout_);
    this->stdio_[2].flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    this->stdio_[2].data.stream = reinterpret_cast<uv_stream_t*>(&this->stderr_);

    // NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)

    this->process_.data = this;
    this->stdout_.data = this;
    this->stderr_.data = this;

    if (uv_spawn(editor->loop_, &this->process_, &this->options_) != 0) {
        // Destroy the process object if it failed to spawn.
        Editor::instance()->destroy_process(this->shared_from_this());
        return false;
    }

    this->doc_->properties_["process_attached"] = true;
    editor->emit_event("process::created", this->shared_from_this());

    uv_read_start(reinterpret_cast<uv_stream_t*>(&this->stdout_), AsyncProcess::on_alloc, AsyncProcess::on_read);
    uv_read_start(reinterpret_cast<uv_stream_t*>(&this->stderr_), AsyncProcess::on_alloc, AsyncProcess::on_read);

    return true;
}

void AsyncProcess::kill() {
    if (uv_is_closing(reinterpret_cast<uv_handle_t*>(&this->process_)) == 0) {
        uv_process_kill(&this->process_, SIGTERM);
    }
}

void AsyncProcess::on_alloc(uv_handle_t* /* handle */, const size_t suggested_size, uv_buf_t* buf) {
    // Allocate buffer for read in AsyncProcess::on_read.
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void AsyncProcess::on_read(uv_stream_t* stream, const ssize_t nread, const uv_buf_t* buf) {
    auto* self{static_cast<AsyncProcess*>(stream->data)};

    if (nread > 0) {
        // Insert text directly into the Document. Since some processes output a lot of text, crossing the C++-Lua
        // boundary for every read could lead to noticable slowdowns.
        self->insert_pos_ = self->ansi_parser_.parse(
            std::string_view(buf->base, nread), self->insert_pos_.value_or(self->doc_->size()));

        Editor::instance()->request_render();
    } else if (nread < 0) {
        // Flush any remaining data.
        self->insert_pos_ = self->ansi_parser_.flush(self->insert_pos_.value_or(self->doc_->size()));
        Editor::instance()->request_render();

        uv_close(reinterpret_cast<uv_handle_t*>(stream), AsyncProcess::on_close);
    }

    // Free allocated buffer from AsyncProcess::on_alloc.
    delete[] buf->base;
}

void AsyncProcess::on_close(uv_handle_t* handle) {
    auto* self{static_cast<AsyncProcess*>(handle->data)};
    self->closed_handles_ += 1;

    // Only destroy the process when all handles are closed.
    if (self->closed_handles_ == 3) {
        Editor::instance()->emit_event("process::exited", self->shared_from_this(), self->exit_status_);
        self->doc_->properties_["process_attached"] = sol::lua_nil;

        Editor::instance()->destroy_process(self->shared_from_this());
    }
}

void AsyncProcess::on_exit(uv_process_t* req, const int64_t status, const int /* signal */) {
    auto* self{static_cast<AsyncProcess*>(req->data)};
    self->exit_status_ = status;

    uv_close(reinterpret_cast<uv_handle_t*>(&self->process_), AsyncProcess::on_close);
}
