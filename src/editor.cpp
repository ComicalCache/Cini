#include "editor.hpp"

#include <uv.h>

#include <ranges>

#include "key.hpp"
#include "lua_defaults.hpp"

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
    // Keybinds.
    auto keybind = this->lua_->create_named_table("Keybind");
    keybind.set_function(
        "bind", [this](const std::string_view mode, const std::string_view key_str, const sol::function& cmd) {
            if (Key key{0, key::Mod::NONE}; Key::try_parse_string(key_str, key)) {
                auto cpp_cmd = [cmd](Editor& self) {
                    // TODO: log error.
                    if (const auto result = cmd(self); !result.valid()) { sol::error err = result; }
                };

                this->get_mode(mode).keymap_[key] = cpp_cmd;
            }
        });

    // Core.
    // clang-format off
    auto core = this->lua_->create_named_table("Core");
    core.new_usertype<Viewport>("Viewport",
        "toggle_gutter", [](Viewport& self) {
            self.gutter_ = !self.gutter_;
        },
        "cursor_up", [](Viewport& self, const std::size_t n = 1) {
            self.move_cursor(&Cursor::up, n);
        },
        "cursor_down", [](Viewport& self, const std::size_t n = 1) {
            self.move_cursor(&Cursor::down, n);
        },
        "cursor_left", [](Viewport& self, const std::size_t n = 1) {
            self.move_cursor(&Cursor::left, n);
        },
        "cursor_right", [](Viewport& self, const std::size_t n = 1) {
            self.move_cursor(&Cursor::right, n);
        },
        "scroll_up", [](Viewport& self, const std::size_t n = 1) {
            self.scroll_up(n);
        },
        "scroll_down", [](Viewport& self, const std::size_t n = 1) {
            self.scroll_down(n);
        },
        "scroll_left", [](Viewport& self, const std::size_t n = 1) {
            self.scroll_left(n);
        },
        "scroll_right", [](Viewport& self, const std::size_t n = 1) {
            self.scroll_right(n);
        });
    core.new_usertype<Editor>("Editor",
        "quit", [](const Editor& self) { uv_stop(self.loop_); },
        "current_viewport", [](Editor& self) -> Viewport& {
            return self.viewports_[self.active_viewport_];
        });
    // clang-format on

    return *this;
}

Editor& Editor::init_state() {
    int width, height;
    uv_tty_get_winsize(&this->tty_out_, &width, &height);
    this->display_.resize(width, height);

    // One Document must always exist.
    this->documents_.push_back(std::make_shared<Document>(std::nullopt));
    // One Viewport must always exist.
    this->viewports_.emplace_back(width, height, this->documents_.back());

    // Init lua with defaults.
    auto result = this->lua_->script(std::string_view(reinterpret_cast<const char*>(lua_defaults::data),
                                                      lua_defaults::len));
    // TODO: log fatal error and exit.
    if (!result.valid()) { sol::error err = result; }

    // Load user config if available.
    if (const auto home = std::getenv("HOME"); home) {
        const auto path = std::filesystem::path{home} / ".config/cini/init.lua";
        if (const auto config = util::read_file(path); config.has_value()) {
            result = this->lua_->script(*config);
            // TODO: log error.
            if (!result.valid()) { sol::error err = result; }
        }
    }

    // Initial render of the editor.
    this->render();

    return *this;
}

void Editor::run() { uv_run(this->loop_, UV_RUN_DEFAULT); }

Mode& Editor::get_mode(std::string_view mode) {
    if (mode == "Global") { return this->global_mode_; }
    if (const auto it = this->mode_registry_.find(mode); it != this->mode_registry_.end()) { return it->second; }

    auto [it, success] = this->mode_registry_.emplace(std::string(mode), Mode{std::string(mode), {}});
    assert(success);
    return it->second;
}

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
        if (Key::try_parse_ansi(self->input_buff_, key)) { // Successful parse.
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
    self->input_buff_.clear();
    self->render();
}

void Editor::render() {
    for (auto& viewport: this->viewports_) { viewport.render(this->display_); }
    this->viewports_[this->active_viewport_].render_cursor(this->display_);
    this->display_.render(&this->tty_out_);
}

void Editor::process_key(const Key key) {
    const auto& doc = this->viewports_[this->active_viewport_].doc_;

    // 1. Check Local Minor Modes.
    for (auto& [name, keymap]: std::ranges::reverse_view(doc->minor_modes_)) {
        if (auto match = keymap.find(key); match != keymap.end()) {
            match->second(*this);
            return;
        }
    }

    // 2. Check Global Minor Modes.
    for (auto& [name, keymap]: std::ranges::reverse_view(this->global_minor_modes_)) {
        if (auto match = keymap.find(key); match != keymap.end()) {
            match->second(*this);
            return;
        }
    }

    // 3. Check Document Major Mode.
    if (const auto match = doc->major_mode_.keymap_.find(key); match != doc->major_mode_.keymap_.end()) {
        match->second(*this);
        return;
    }

    // 4. Check Global Mode.
    if (const auto match = this->global_mode_.keymap_.find(key); match != this->global_mode_.keymap_.end()) {
        match->second(*this);
    }
}
