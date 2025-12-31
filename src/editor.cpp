#include "editor.hpp"

#include <ranges>

#include "direction.hpp"
#include "document.hpp"
#include "key.hpp"
#include "key_special.hpp"
#include "lua_defaults.hpp"
#include "util.hpp"
#include "version.hpp"
#include "viewport.hpp"
#include "window.hpp"

void Editor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Editor>("Editor",
        "active_viewport", sol::property([](const Editor& editor) { return editor.active_viewport_; }),
        "quit", [](Editor& editor) { editor.close_active_viewport(); },
        "get_mode", &Editor::get_mode,
        "set_global_mode", [](Editor& editor, const Mode& mode) { editor.global_mode_ = mode; },
        "add_global_minor_mode", [](Editor& editor, const Mode& mode) { editor.global_minor_modes_.push_back(mode); },
        "remove_global_minor_mode", [](Editor& editor, const std::string& name) {
          std::erase_if(editor.global_minor_modes_, [&](const Mode& mode) { return mode.name_ == name; });
        },
        "split_vertical", [](Editor& editor) { editor.split_active_viewport(true); },
        "split_horizontal", [](Editor& editor) { editor.split_active_viewport(false); },
        "resize_split", [](Editor& editor, const float delta) { editor.resize_active_viewport_split(delta); },
        "navigate", &Editor::navigate_window,
        "next_key", [](Editor& self, const sol::function& cmd) {
            self.input_handler_ = [cmd](Editor& editor, Key key) {
                if (cmd.valid()) { cmd(editor, key); }
            };
        });
    // clang-format on
}

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

std::optional<Face> Editor::resolve_face(const std::string_view face, const Viewport& viewport) const {
    assert(viewport.doc_);

    std::optional<Face> res = std::nullopt;

    // 1. Check Global Mode Faces.
    if (const auto it = this->global_mode_.faces_.find(face); it != this->global_mode_.faces_.end()) {
        res = it->second;
    }

    // 2. Check Major Mode Faces.
    const auto& major = viewport.doc_->major_mode_;
    if (const auto it = major.faces_.find(face); it != major.faces_.end()) {
        res.and_then([it](auto r) -> std::optional<Face> {
            r.merge(it->second);
            return r;
        });
    }

    // 3. Check Global Minor Mode faces.
    for (const auto& mode: std::ranges::reverse_view(this->global_minor_modes_)) {
        if (auto it = mode.faces_.find(face); it != mode.faces_.end()) {
            res.and_then([it](auto r) -> std::optional<Face> {
                r.merge(it->second);
                return r;
            });
        }
    }

    // 4. Check Document Minor Mode faces.
    for (const auto& mode: std::ranges::reverse_view(viewport.doc_->minor_modes_)) {
        if (auto it = mode.faces_.find(face); it != mode.faces_.end()) {
            res.and_then([it](auto r) -> std::optional<Face> {
                r.merge(it->second);
                return r;
            });
        }
    }

    return res;
}

std::optional<Replacement> Editor::resolve_replacement(const std::string_view ch, const Viewport& viewport) const {
    assert(viewport.doc_);

    std::optional<Replacement> res{};

    // 1. Check Global Mode Replacements.
    if (const auto it = this->global_mode_.replacements_.find(ch); it != this->global_mode_.replacements_.end()) {
        res = it->second;
    }

    // 2. Check Major Mode Replacements.
    const auto& major = viewport.doc_->major_mode_;
    if (const auto it = major.replacements_.find(ch); it != major.replacements_.end()) { res = it->second; }

    // 3. Check Global Minor Mode Replacements.
    for (const auto& mode: std::ranges::reverse_view(this->global_minor_modes_)) {
        if (auto it = mode.replacements_.find(ch); it != mode.replacements_.end()) { res = it->second; }
    }

    // 4. Check Document Minor Mode Replacements.
    for (const auto& mode: std::ranges::reverse_view(viewport.doc_->minor_modes_)) {
        if (auto it = mode.replacements_.find(ch); it != mode.replacements_.end()) { res = it->second; }
    }

    return res;
}

Mode& Editor::get_mode(const std::string_view mode) {
    if (mode == "global") { return this->global_mode_; }
    if (const auto it = this->mode_registry_.find(mode); it != this->mode_registry_.end()) { return *it->second; }

    auto [it, success] = this->mode_registry_.emplace(std::string(mode),
                                                      std::make_unique<Mode>(
                                                          std::string(mode), Keymap{}, nullptr, ReplacementMap{},
                                                          FaceMap{}));
    assert(success);
    return *it->second;
}

const std::vector<Mode>& Editor::get_global_minor_modes() const { return this->global_minor_modes_; }

void Editor::split_active_viewport(bool vertical) {
    auto new_viewport = std::make_shared<Viewport>(*this->active_viewport_);
    auto new_leaf = std::make_shared<Window>(new_viewport);

    // No Split exists yet.
    if (this->window_->viewport_ == this->active_viewport_) {
        // Old will be left, new will be right.
        this->window_ = std::make_shared<Window>(this->window_, new_leaf, vertical);
        this->active_viewport_ = new_viewport;

        // Calculate dimensions.
        resize(&this->sigwinch_, 0);

        return;
    }

    auto [parent, child] = this->window_->find_parent(this->active_viewport_);
    auto old_leaf = child == 1 ? parent->child_1_ : parent->child_2_;
    // Old will be left, new will be right.
    const auto new_split = std::make_shared<Window>(old_leaf, new_leaf, vertical);

    if (child == 1) { parent->child_1_ = new_split; } else { parent->child_2_ = new_split; }

    this->active_viewport_ = new_viewport;

    resize(&this->sigwinch_, 0);
}

void Editor::resize_active_viewport_split(const float delta) {
    if (this->window_->viewport_ == this->active_viewport_) { return; }

    // FIXME: replace _N with _ when upgrading to C++26.
    auto [parent, _1] = this->window_->find_parent(this->active_viewport_);
    parent->ratio_ = std::clamp(parent->ratio_ + delta, 0.1f, 0.9f);
    resize(&this->sigwinch_, 0);
}

void Editor::close_active_viewport() {
    if (this->window_->viewport_) { // Single Window, quit Editor.
        quit(&this->sigquit_, 0);
    } else {
        auto [parent, child] = this->window_->find_parent(this->active_viewport_);
        const auto new_node = child == 1 ? parent->child_2_ : parent->child_1_;

        *parent = *new_node;

        if (new_node->viewport_) { // New node is a leaf.
            this->active_viewport_ = new_node->viewport_;
        } else { this->active_viewport_ = util::find_viewport(new_node); }

        resize(&this->sigwinch_, 0);
    }
}

void Editor::navigate_window(Direction direction) {
    std::vector<std::pair<Window*, std::size_t>> path;

    if (!this->window_->get_path(this->active_viewport_, path)) { return; }

    for (auto& [window, child]: std::ranges::reverse_view(path)) {
        bool can_move = false;
        std::size_t idx = 0;
        const auto is_vert = window->vertical_;

        switch (direction) {
            case Direction::LEFT: {
                if (!is_vert && child == 2) {
                    can_move = true;
                    idx = 1;
                }
                break;
            }
            case Direction::RIGHT: {
                if (!is_vert && child == 1) {
                    can_move = true;
                    idx = 2;
                }
                break;
            }
            case Direction::UP: {
                if (is_vert && child == 2) {
                    can_move = true;
                    idx = 1;
                }
                break;
            }
            case Direction::DOWN: {
                if (is_vert && child == 1) {
                    can_move = true;
                    idx = 2;
                }
                break;
            }
        }

        if (can_move) {
            const auto sibling = idx == 1 ? window->child_1_ : window->child_2_;
            const auto prefer_first = direction == Direction::RIGHT || direction == Direction::DOWN;

            this->active_viewport_ = sibling->edge_leaf(prefer_first);
            return;
        }
    }
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
                               sol::lib::jit, sol::lib::table);

    // Add a loader for predefined defaults.
    sol::table loaders = (*this->lua_)["package"]["loaders"];
    loaders.add([this](const std::string& name) -> sol::optional<sol::function> {
        const auto it = lua_modules::files.find(name);
        if (it == lua_modules::files.end()) { return sol::nullopt; }

        const sol::load_result res = this->lua_->load(it->second, name);
        if (!res.valid()) {
            // TODO: log errors.
            return sol::nullopt;
        }

        return res.get<sol::function>();
    });

    return *this;
}

Editor& Editor::init_bridge() {
    auto core = this->lua_->create_named_table("Core");
    auto keybind = this->lua_->create_named_table("Keybind");

    Cursor::init_bridge(core);
    Document::init_bridge(*this, core);
    Editor::init_bridge(core);
    Face::init_bridge(core);
    Key::init_bridge(core);
    Mode::init_bridge(*this, core, keybind);
    Rgb::init_bridge(core);
    Viewport::init_bridge(core);
    Window::init_bridge(core);

    // clang-format off
    // Phantom struct to declare read only state to Lua.
    struct State {};
    this->lua_->new_usertype<State>("State",
        "editor", sol::property([this](const State&) { return this; }),
        "name", sol::property([](const State&) { return version::NAME; }),
        "version", sol::property([](const State&) { return version::VERSION; }),
        "build_date", sol::property([](const State&) { return version::BUILD_DATE; }),
        "build_type", sol::property([](const State&) { return version::BUILD_TYPE; }));
    this->lua_->set("State", State{});
    // clang-format on

    return *this;
}

Editor& Editor::init_state() {
    int width{}, height{};
    uv_tty_get_winsize(&this->tty_out_, &width, &height);
    this->display_.resize(width, height);

    // One Document must always exist.
    this->documents_.push_back(std::make_shared<Document>(std::nullopt));

    // TODO: remove
    // clang-format off
    this->documents_.back()->insert(0,
        "123456781234567812345678\n"
        "------------------------\n"
        "Tab Test:\n"
        "\tStart\n"
        "a\tAlign 4\n"
        "ab\tAlign 4\n"
        "abc\tAlign 4\n"
        "abcd\tAlign 8\n"
        "\n""Wide Char Test:\n"
        "ASCII:    |..|..|\n"
        "Chinese:  |你 好|\n"
        "Mixed:    |a你b好|\n"
        "Emoji:    |😀|\n"
        "Missing: �\n"
        "Trailing whitespace:    \n"
        "Trailing tabs:  \t\n"
        "\n""Edge Cases:\n"
        "\t\tDouble Tab\n"
        "你\tWide+Tab\n"
        "Line with CRLF\r\n"
        "\n"
        "End");
    // clang-format on

    // One Viewport must always exist.
    this->active_viewport_ = std::make_shared<Viewport>(width, height, this->documents_.back());

    // Create Window tree.
    this->window_ = std::make_shared<Window>(this->active_viewport_);

    // Init lua with defaults.
    auto result = this->lua_->script("require('init')");

    // TODO: log fatal error and exit.
    if (!result.valid()) {
        sol::error err = result;
        exit(1);
    }

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
    while (true) {
        if (auto key = Key::try_parse_ansi(self->input_buff_); key) { // Successful parse.
            self->process_key(*key);
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
    // TODO: log error?
    if (uv_tty_get_winsize(&self->tty_out_, &width, &height) != 0) { return; }

    self->window_->resize(0, 0, width, height);
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
    self->process_key(Key{static_cast<std::size_t>(KeySpecial::ESCAPE), KeyMod::NONE});
    // If this callback is called, input_buff_ only contains a single Esc key and can be safely cleared.
    self->input_buff_.clear();
    self->render();
}

void Editor::render() {
    this->window_->render(this->display_, *this);
    this->active_viewport_->render_cursor(this->display_);
    this->display_.render(&this->tty_out_);
}

void Editor::process_key(const Key key) {
    if (this->input_handler_) {
        const auto handler = std::move(this->input_handler_);
        this->input_handler_ = nullptr;
        handler(*this, key);
        return;
    }

    // Copy of std::shared_ptr<Document> to keep it alive even if it gets removed from the Viewport.
    const auto& doc = this->active_viewport_->doc_;

    // Safely execute a command.
    auto execute = [&](const Command& cmd) {
        const auto copy = cmd;
        copy(*this);
    };
    auto execute_catch_all = [&](const CatchAllCommand& cmd, const Key key) {
        const auto copy = cmd;
        return copy(*this, key);
    };

    // 1. Check Local Minor Modes.
    // FIXME: replace _N with _ when upgrading to C++26.
    for (auto& [_1, keymap, catch_all, _2, _3, _4]: std::ranges::reverse_view(doc->minor_modes_)) {
        if (auto match = keymap.find(key); match != keymap.end()) {
            execute(match->second);
            return;
        }
        if (catch_all && execute_catch_all(catch_all, key)) { return; }
    }

    // 2. Check Global Minor Modes.
    // FIXME: replace _N with _ when upgrading to C++26.
    for (auto& [_1, keymap, catch_all, _2, _3, _4]: std::ranges::reverse_view(this->global_minor_modes_)) {
        if (auto match = keymap.find(key); match != keymap.end()) {
            execute(match->second);
            return;
        }
        if (catch_all && execute_catch_all(catch_all, key)) { return; }
    }

    // 3. Check Document Major Mode.
    if (const auto match = doc->major_mode_.keymap_.find(key); match != doc->major_mode_.keymap_.end()) {
        execute(match->second);
        return;
    }
    if (doc->major_mode_.catch_all_ && doc->major_mode_.catch_all_(*this, key)) { return; }

    // 4. Check Global Mode.
    if (const auto match = this->global_mode_.keymap_.find(key); match != this->global_mode_.keymap_.end()) {
        execute(match->second);
        return;
    }
    if (this->global_mode_.catch_all_ && this->global_mode_.catch_all_(*this, key)) { return; }
}
