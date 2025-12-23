#include "editor.hpp"

#include <uv.h>

#include "key.hpp"

Editor::Editor() : lua_{std::make_unique<sol::state>()}, loop_{uv_default_loop()} {}

Editor::~Editor() {
    uv_tty_reset_mode();

    uv_close(reinterpret_cast<uv_handle_t*>(&this->tty_in_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->tty_out_), nullptr);

    uv_close(reinterpret_cast<uv_handle_t*>(&this->sigwinch_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->sigint_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->sigquit_), nullptr);

    // Drain loop of handle close events.
    while (uv_loop_alive(this->loop_)) { uv_run(this->loop_, UV_RUN_NOWAIT); }
    uv_loop_close(this->loop_);
}

Editor& Editor::init_uv() {
    uv_tty_init(this->loop_, &this->tty_in_, 0, 1);
    uv_tty_init(this->loop_, &this->tty_out_, 1, 0);
    uv_tty_set_mode(&this->tty_in_, UV_TTY_MODE_RAW);
    // Store instance in the write request to have access to this in callback.
    this->tty_in_.data = this;
    // Store instance in the write request to have access to this in callback.
    this->tty_out_.data = this;
    uv_read_start(reinterpret_cast<uv_stream_t*>(&this->tty_in_), &Editor::alloc_input, &Editor::input);

    uv_signal_init(this->loop_, &this->sigwinch_);
    uv_signal_init(this->loop_, &this->sigint_);
    uv_signal_init(this->loop_, &this->sigquit_);
    // Store instance in the write request to have access to this in callback.
    this->sigwinch_.data = this;
    // Store instance in the write request to have access to this in callback.
    this->sigint_.data = this;
    // Store instance in the write request to have access to this in callback.
    this->sigquit_.data = this;
    uv_signal_start(&this->sigwinch_, &Editor::resize, SIGWINCH);
    uv_signal_start(&this->sigint_, &Editor::quit, SIGINT);
    uv_signal_start(&this->sigquit_, &Editor::quit, SIGQUIT);

    uv_timer_init(this->loop_, &this->esc_timer_);
    // Store instance in the write request to have access to this in callback.
    this->esc_timer_.data = this;

    return *this;
}

Editor& Editor::init_lua() {
    this->lua_->open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::os, sol::lib::io,
                               sol::lib::jit);

    return *this;
}

Editor& Editor::init_bridge() {
    // TODO
    return *this;
}

Editor& Editor::init_state() {
    int width, height;
    uv_tty_get_winsize(&this->tty_out_, &width, &height);
    this->display_.resize(width, height);

    // One Document must always exist.
    this->documents_.push_back(std::make_shared<Document>(std::nullopt));

    // TODO: remove
    this->documents_.back()->insert(
        0, "123456781234567812345678  (Ruler)\n""------------------------\n""Tab Test:\n""\tStart\n""a\tAlign 4\n"
        "ab\tAlign 4\n""abc\tAlign 4\n""abcd\tAlign 8\n""\n""Wide Char Test:\n""ASCII:    |..|..|\n""Chinese:  |你 好|\n"
        "Mixed:    |a你b好|\n""Emoji:    |😀| (Might be 2 or 1 depending on terminal)\n""\n""Edge Cases:\n"
        "\t\tDouble Tab\n""你\tWide+Tab\n""Line with CRLF\r\n""\n""End");

    // One Viewport must always exist.
    this->viewports_.emplace_back(width, height, this->documents_.back());

    // Initial render of the editor.
    this->render();

    return *this;
}

void Editor::run() { uv_run(this->loop_, UV_RUN_DEFAULT); }

void Editor::alloc_input(uv_handle_t*, size_t, uv_buf_t* buf) {
    // Large static input buffer to avoid memory allocation and frees.
    static char input_buffer[4096];
    buf->base = input_buffer;
    buf->len = sizeof(input_buffer);
}

void Editor::input(uv_stream_t* stream, const ssize_t nread, const uv_buf_t* buf) {
    auto* self = static_cast<Editor*>(stream->data);

    if (nread < 0) {
        if (nread != UV_EOF) { /* TODO: log error. */ }
        return;
    }

    // Stop Esc waiting timers since new data arrived.
    uv_timer_stop(&self->esc_timer_);

    // Append new data to the input buffer. This handles partially received sequences.
    self->input_buff_.append(buf->base, nread);

    // Consume as many keys as possible.
    Key key{0, key::Mod::NONE};
    while (true) {
        if (Key::try_parse(self->input_buff_, key)) { // Successful parse.
            self->process_key(key);
        } else if (self->input_buff_.size() == 1 && self->input_buff_[0] == '\x1b') { // Lone Esc.
            uv_timer_start(&self->esc_timer_, &Editor::esc_timer, 20, 0);
            break;
        } else { break; }
    }

    // Render after received inputs.
    self->render();
}

void Editor::resize(uv_signal_t* handle, int) {
    auto* self = static_cast<Editor*>(handle->data);

    int width, height;
    uv_tty_get_winsize(&self->tty_out_, &width, &height);

    // TODO: fix offset calculation.
    for (auto& viewport: self->viewports_) { viewport.resize(width, height, Position{}); }
    self->display_.resize(width, height);

    // Render after resizing.
    self->render();
}

void Editor::quit(uv_signal_t* handle, int) {
    const auto* self = static_cast<Editor*>(handle->data);

    // TODO: don't stop but show message to use the quit command.
    uv_stop(self->loop_);
}

void Editor::esc_timer(uv_timer_t* handle) {
    auto* self = static_cast<Editor*>(handle->data);
    self->process_key(Key{static_cast<std::size_t>(key::Special::ESCAPE), key::Mod::NONE});
    self->render();
}

void Editor::render() {
    this->viewports_[this->active_viewport_].render(this->display_);
    this->viewports_[this->active_viewport_].render_cursor(this->display_);
    this->display_.render(&this->tty_out_);
}

void Editor::process_key(const Key key) {
    // TODO: handle keys.
    // 1. Try Minor Mode keys.
    // 2. Try Major Mode keys.
    // 3. Try Global Mode keys.

    // TODO: remove
    if (key == Key{'q', key::Mod::CTRL}) { uv_stop(this->loop_); }
}
