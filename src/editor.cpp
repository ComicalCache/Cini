#include "editor.hpp"

#include <ranges>

#include "document.hpp"
#include "gen/lua_defaults.hpp"
#include "gen/version.hpp"
#include "key.hpp"
#include "regex.hpp"
#include "typedef/key_special.hpp"
#include "types/direction.hpp"
#include "types/face.hpp"
#include "util/assert.hpp"
#include "util/fs.hpp"
#include "viewport.hpp"
#include "window.hpp"

void Editor::setup(const std::optional<std::filesystem::path>& path) {
    Editor::instance().lock()->init_uv().init_lua().init_bridge().init_state(path);
    Editor::instance().lock()->initialized_ = true;
}

void Editor::run() { uv_run(Editor::instance().lock()->loop_, UV_RUN_DEFAULT); }

void Editor::destroy() {
    if (const auto self = Editor::instance().lock()) { self->shutdown(); }
}

std::weak_ptr<Editor> Editor::instance() {
    struct PublicEditor : Editor {};

    static std::shared_ptr<PublicEditor> editor{nullptr};

    if (!editor) { editor = std::make_unique<PublicEditor>(); }

    return editor;
}

Editor::Editor() : lua_{std::make_unique<sol::state>()}, loop_{uv_default_loop()}, mini_buffer_{0, 0, *this->lua_} {}
Editor::~Editor() { this->shutdown(); }

Editor& Editor::init_uv() {
    // Stores the instance in the handle to have access to it in the callback like the following:
    // this->uv_handle_.data = this;

    uv_tty_init(this->loop_, &this->tty_in_, 0, 1);
    uv_tty_init(this->loop_, &this->tty_out_, 1, 0);
    uv_tty_set_mode(&this->tty_in_, UV_TTY_MODE_RAW);
    this->tty_in_.data = this;
    this->tty_out_.data = this;
    uv_read_start(reinterpret_cast<uv_stream_t*>(&this->tty_in_), &Editor::alloc_input, &Editor::input);

    uv_signal_init(this->loop_, &this->sigwinch_);
    uv_signal_init(this->loop_, &this->sigint_);
    uv_signal_init(this->loop_, &this->sigquit_);
    this->sigwinch_.data = this;
    this->sigint_.data = this;
    this->sigquit_.data = this;
    uv_signal_start(&this->sigwinch_, &Editor::resize, SIGWINCH);
    uv_signal_start(&this->sigint_, &Editor::quit, SIGINT);
    uv_signal_start(&this->sigquit_, &Editor::quit, SIGQUIT);

    uv_timer_init(this->loop_, &this->esc_timer_);
    uv_timer_init(this->loop_, &this->status_message_timer_);
    this->esc_timer_.data = this;
    this->status_message_timer_.data = this;

    return *this;
}

Editor& Editor::init_lua() {
    this->lua_->open_libraries(
        sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::os, sol::lib::io, sol::lib::table,
        sol::lib::debug);

    // Handle Lua panic.
    this->lua_->set_panic([](lua_State* L) -> int {
        std::string s{};

        ansi::main_screen(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << lua_tostring(L, -1) << std::endl;

        uv_tty_reset_mode();
        exit(1);
    });
    // Generate trace on Lua errors.
    this->lua_->set_function("__panic", [](const sol::this_state L, const std::string& msg) -> std::string {
        return sol::state_view{L}["debug"]["traceback"](msg, 2);
    });
    sol::protected_function::set_default_handler((*this->lua_)["__panic"]);

    // Add a loader for predefined defaults.
    sol::table loaders = (*this->lua_)["package"]["searchers"];
    loaders.add([](const sol::this_state L, const std::string& name) -> sol::optional<sol::function> {
        const auto it = lua_modules::files.find(name);
        if (it == lua_modules::files.end()) { return sol::nullopt; }

        const sol::load_result res = sol::state_view{L}.load(it->second, name);
        if (!res.valid()) {
            std::string s{};

            ansi::main_screen(s);
            std::print("{}", s);
            std::fflush(stdout);
            std::cerr << "Couldn't find module '" << name << "'." << std::endl;

            uv_tty_reset_mode();
            exit(1);
        }

        return res.get<sol::function>();
    });

    return *this;
}

Editor& Editor::init_bridge() {
    auto core = this->lua_->create_named_table("Core");

    Cursor::init_bridge(core);
    direction::init_bridge(core);
    Document::init_bridge(core);
    Editor::init_bridge(core);
    Face::init_bridge(core);
    Key::init_bridge(core);
    Regex::init_bridge(core);
    RegexMatch::init_bridge(core);
    Rgb::init_bridge(core);
    Viewport::init_bridge(core);

    // clang-format off
    // Phantom struct to declare read-only state to Lua.
    struct State {};
    this->lua_->new_usertype<State>("State",
        "editor", sol::property([this](const State&) { return std::ref(*this); }),
        "name", sol::property([](const State&) { return version::NAME; }),
        "version", sol::property([](const State&) { return version::VERSION; }),
        "build_date", sol::property([](const State&) { return version::BUILD_DATE; }),
        "build_type", sol::property([](const State&) { return version::BUILD_TYPE; }));
    this->lua_->set("State", State{});
    // clang-format on

    return *this;
}

Editor& Editor::init_state(const std::optional<std::filesystem::path>& path) {
    // Init lua with defaults.
    if (const auto result = this->lua_->safe_script("require('init')"); !result.valid()) {
        sol::error err = result;
        std::string s{};

        ansi::main_screen(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << err.what() << std::endl;

        uv_tty_reset_mode();
        exit(1);
    }

    // Defer user config errors until the setup is complete.
    std::optional<sol::error> user_config_result{std::nullopt};

    // Load user config if available.
    if (const auto home = std::getenv("HOME"); home) {
        const auto user_config = std::filesystem::path{home} / ".config/cini/init.lua";
        if (const auto config = fs::read_file(user_config); config.has_value()) {
            if (const auto result = this->lua_->safe_script(*config); !result.valid()) { user_config_result = result; }
        }
    }

    int width{}, height{};
    uv_tty_get_winsize(&this->tty_out_, &width, &height);

    this->mini_buffer_ = MiniBuffer(width, 1, *this->lua_);
    // One Document must always exist.
    this->documents_.push_back(std::make_shared<Document>(path, *this->lua_));
    // One Viewport must always exist.
    this->active_viewport_ = std::make_shared<Viewport>(width, height, this->documents_.back());
    this->window_ = std::make_shared<Window>(this->active_viewport_);

    // Init lua with defaults after initialization.
    if (const auto result = this->lua_->safe_script("require('post_init')"); !result.valid()) {
        sol::error err = result;
        std::string s{};

        ansi::main_screen(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << err.what() << std::endl;

        uv_tty_reset_mode();
        exit(1);
    }

    // Load user config after initialization if available.
    if (const auto home = std::getenv("HOME"); !user_config_result && home) {
        const auto user_config = std::filesystem::path{home} / ".config/cini/post_init.lua";
        if (const auto config = fs::read_file(user_config); config.has_value()) {
            if (const auto result = this->lua_->safe_script(*config); !result.valid()) { user_config_result = result; }
        }
    }

    // Initial render of the editor.
    // this->is_rendering_ is true to avoid errors during state initialization to be rendered before setup is completed.
    // Set it to false now.
    this->is_rendering_ = false;
    resize(&this->sigwinch_, 0);
    if (user_config_result) {
        // TODO: log error.
    } else {
        this->render();
    }

    return *this;
}

void Editor::shutdown() {
    if (!this->initialized_) { return; }

    uv_tty_reset_mode();

    uv_close(reinterpret_cast<uv_handle_t*>(&this->tty_in_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->tty_out_), nullptr);

    uv_close(reinterpret_cast<uv_handle_t*>(&this->sigwinch_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->sigint_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->sigquit_), nullptr);

    uv_close(reinterpret_cast<uv_handle_t*>(&this->esc_timer_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&this->status_message_timer_), nullptr);

    // Drain loop of handle close events.
    while (uv_loop_alive(this->loop_)) { uv_run(this->loop_, UV_RUN_NOWAIT); }
    uv_loop_close(this->loop_);

    this->lua_.reset();

    this->initialized_ = false;
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
        if (nread != UV_EOF) {
            // TODO: log error.
        }
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
        } else {
            break;
        }
    }

    self->render();
}

void Editor::resize(uv_signal_t* handle, const int mode) {
    auto* self = static_cast<Editor*>(handle->data);

    int width{};
    int height{};
    if (uv_tty_get_winsize(&self->tty_out_, &width, &height) != 0) { return; }

    self->display_.resize(width, height);

    if (const auto mb_height = self->mini_buffer_.viewport_->height_; static_cast<std::size_t>(height) > mb_height) {
        height -= static_cast<int>(mb_height);
    }
    self->window_->resize(0, 0, width, height);
    self->mini_buffer_.viewport_->resize(width, 1, Position{static_cast<std::size_t>(height), 0});

    if (mode != 0) { self->render(); }
}

void Editor::quit(uv_signal_t* handle, int) {
    // TODO: remove.
    (void)handle;
    // auto* self = static_cast<Editor*>(handle->data);
    // TODO: log info to use proper quit command.
}

void Editor::esc_timer(uv_timer_t* handle) {
    auto* self = static_cast<Editor*>(handle->data);
    self->process_key(Key{static_cast<std::size_t>(KeySpecial::ESCAPE), KeyMod::NONE});
    // If this callback is called, input_buff_ only contains a single Esc key and can be safely cleared.
    self->input_buff_.clear();
    self->render();
}

void Editor::status_message_timer(uv_timer_t* handle) {
    auto* self = static_cast<Editor*>(handle->data);
    self->mini_buffer_.viewport_->doc_->clear();
    self->render();
}

void Editor::enter_mini_buffer() {
    if (this->active_viewport_ == this->mini_buffer_.viewport_) { return; }

    uv_timer_stop(&this->status_message_timer_);

    this->mini_buffer_.prev_viewport_ = this->active_viewport_;
    this->active_viewport_ = this->mini_buffer_.viewport_;
}

void Editor::exit_mini_buffer() {
    if (this->active_viewport_ != this->mini_buffer_.viewport_) { return; }

    ASSERT(this->mini_buffer_.prev_viewport_ != nullptr, "");

    this->active_viewport_ = this->mini_buffer_.prev_viewport_;
    this->mini_buffer_.prev_viewport_ = nullptr;
}

void Editor::split_viewport(bool vertical, const float ratio) {
    // Don't split if in mini buffer.
    if (this->active_viewport_ == this->mini_buffer_.viewport_) { return; }

    if ((vertical && this->active_viewport_->height_ < 8) || (!vertical && this->active_viewport_->width_ < 25)) {
        return;
    }

    auto new_viewport = std::make_shared<Viewport>(*this->active_viewport_);
    auto new_leaf = std::make_shared<Window>(new_viewport);

    // No Split exists yet.
    if (this->window_->viewport_ == this->active_viewport_) {
        // Old will be left, new will be right.
        this->window_ = std::make_shared<Window>(this->window_, new_leaf, vertical);
        this->window_->ratio_ = ratio;
        this->active_viewport_ = new_viewport;

        // Calculate dimensions.
        resize(&this->sigwinch_, 0);

        return;
    }

    auto [parent, child] = this->window_->find_parent(this->active_viewport_);
    auto old_leaf = child == 1 ? parent->child_1_ : parent->child_2_;
    // Old will be left, new will be right.
    const auto new_split = std::make_shared<Window>(old_leaf, new_leaf, vertical);
    new_split->ratio_ = ratio;

    if (child == 1) {
        parent->child_1_ = new_split;
    } else {
        parent->child_2_ = new_split;
    }

    this->active_viewport_ = new_viewport;

    resize(&this->sigwinch_, 0);
}

void Editor::resize_viewport(const float delta) {
    if (this->active_viewport_ == this->mini_buffer_.viewport_) { return; }
    // Only one full-sized Viewport exists.
    if (this->active_viewport_ == this->window_->viewport_) { return; }

    // FIXME: replace _N with _ when upgrading to C++26.
    auto [parent, _1] = this->window_->find_parent(this->active_viewport_);
    parent->ratio_ = std::clamp(parent->ratio_ + delta, 0.1f, 0.9f);
    resize(&this->sigwinch_, 0);
}

void Editor::close_viewport() {
    if (this->window_->viewport_) { // Single Window, quit Editor.
        uv_stop(this->loop_);
    } else {
        auto [parent, child] = this->window_->find_parent(this->active_viewport_);
        const auto new_node = child == 1 ? parent->child_2_ : parent->child_1_;

        *parent = *new_node;

        if (new_node->viewport_) {
            this->active_viewport_ = new_node->viewport_;
        } else {
            this->active_viewport_ = Viewport::find_viewport(new_node);
        }

        resize(&this->sigwinch_, 0);
    }
}

void Editor::navigate_window(const Direction direction) {
    // Don't navigate if in mini buffer.
    if (this->active_viewport_ == this->mini_buffer_.viewport_) { return; }

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

void Editor::render() {
    if (this->is_rendering_) {
        this->request_rendering_ = true;
    } else {
        this->_render();
    }
}

void Editor::_render() {
    if (this->is_rendering_) { return; }

    this->is_rendering_ = true;

    do {
        this->request_rendering_ = false;

        if (!this->window_->render(this->display_)) { continue; }
        if (!this->mini_buffer_.viewport_->render(this->display_)) { continue; }
        this->active_viewport_->render_cursor(this->display_);
        this->display_.render(&this->tty_out_);
    } while (this->request_rendering_ == true);

    this->is_rendering_ = false;
}

void Editor::process_key(const Key key) {
    if (auto on_input = (*this->lua_)["Core"]["on_input"]; !on_input.valid()) {
        // TODO: log error.
    } else if (const auto result = on_input(*this, key); !result.valid()) {
        const sol::error err = result;
        // TODO: log error.
    }
}
