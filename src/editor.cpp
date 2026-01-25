#include "editor.hpp"

#include <memory>
#include <optional>
#include <ranges>
#include <utility>

#include "document.hpp"
#include "gen/lua_defaults.hpp"
#include "gen/version.hpp"
#include "key.hpp"
#include "regex.hpp"
#include "rendering/window.hpp"
#include "types/direction.hpp"
#include "types/face.hpp"
#include "types/key_mod.hpp"
#include "types/key_special.hpp"
#include "util/fs.hpp"
#include "viewport.hpp"

void Editor::setup(const std::optional<std::filesystem::path>& path) {
    const auto self = Editor::instance();
    self->init_uv().init_lua().init_bridge().init_state(path);
    self->emit_event("cini::startup");
    self->initialized_ = true;
}

void Editor::run() { uv_run(Editor::instance()->loop_, UV_RUN_DEFAULT); }

void Editor::destroy() {
    const auto self = Editor::instance();
    self->emit_event("cini::shutdown");
    self->shutdown();
    self->initialized_ = false;
}

auto Editor::instance() -> std::shared_ptr<Editor> {
    static std::shared_ptr<Editor> editor{nullptr};
    if (!editor) { editor = std::make_unique<Editor>(Editor::EditorKey{}); }

    return editor;
}

Editor::Editor(Editor::EditorKey /* key */)
    : lua_{std::make_unique<sol::state>()}, loop_{uv_default_loop()}, mini_buffer_{0, 0, *this->lua_} {}
Editor::~Editor() { this->shutdown(); }

auto Editor::create_document(std::optional<std::filesystem::path> path) -> std::shared_ptr<Document> {
    auto doc = this->documents_.emplace_back(std::make_shared<Document>(std::move(path), *this->lua_));
    this->emit_event("document::created", doc);
    return doc;
}

auto Editor::create_viewport(std::size_t width, std::size_t height, std::shared_ptr<Document> doc)
    -> std::shared_ptr<Viewport> {
    auto viewport = std::make_shared<Viewport>(width, height, std::move(doc));
    this->emit_event("viewport::created", viewport);
    return viewport;
}

auto Editor::create_viewport(const std::shared_ptr<Viewport>& viewport) -> std::shared_ptr<Viewport> {
    auto new_viewport = std::make_shared<Viewport>(*viewport);
    this->emit_event("viewport::created", viewport);
    return new_viewport;
}

auto Editor::init_uv() -> Editor& {
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

auto Editor::init_lua() -> Editor& {
    this->lua_->open_libraries(
        sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::os, sol::lib::io, sol::lib::table,
        sol::lib::debug);

    // Handle Lua panic.
    this->lua_->set_panic([](lua_State* L) -> int {
        std::string s{};

        ansi::main_screen(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << lua_tostring(L, -1) << "\n";

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
        if (!res.valid()) { return sol::nullopt; }

        return res.get<sol::function>();
    });

    return *this;
}

auto Editor::init_bridge() -> Editor& {
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
        "editor", sol::property([this](const State&) -> std::reference_wrapper<Editor> { return std::ref(*this); }),
        "name", sol::property([](const State&) -> std::string { return version::NAME; }),
        "version", sol::property([](const State&) -> std::string { return version::VERSION; }),
        "build_date", sol::property([](const State&) -> std::string { return version::BUILD_DATE; }),
        "build_type", sol::property([](const State&) -> std::string { return version::BUILD_TYPE; }));
    this->lua_->set("State", State{});
    // clang-format on

    return *this;
}

auto Editor::init_state(const std::optional<std::filesystem::path>& path) -> Editor& {
    // Init lua with defaults.
    if (const auto result = this->lua_->safe_script("require('init')"); !result.valid()) {
        sol::error err = result;
        std::string s{};

        ansi::main_screen(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << err.what() << "\n";

        uv_tty_reset_mode();
        exit(1);
    }

    // Defer user config errors until the setup is complete.
    std::optional<sol::error> user_config_result{std::nullopt};

    // Load user config if available.
    if (const auto* const home = std::getenv("HOME"); home) {
        const auto user_config = std::filesystem::path{home} / ".config/cini/init.lua";
        if (const auto config = fs::read_file(user_config); config.has_value()) {
            if (const auto result = this->lua_->safe_script(*config); !result.valid()) { user_config_result = result; }
        }
    }

    this->mini_buffer_ = MiniBuffer(1, 1, *this->lua_);
    this->emit_event("mini_buffer::created", this->mini_buffer_.viewport_);

    // One Document and Viewport must always exist.
    this->window_manager_.set_root(this->create_viewport(1, 1, this->create_document(path)));

    // Initial render of the editor.
    // this->is_rendering_ is true to avoid errors during state initialization to be rendered before setup is completed.
    // Set it to false now.
    this->is_rendering_ = false;
    resize(&this->sigwinch_, 0);
    if (user_config_result) {
        // TODO: log error.
    }

    this->render();

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
    while (uv_loop_alive(this->loop_) != 0) { uv_run(this->loop_, UV_RUN_NOWAIT); }
    uv_loop_close(this->loop_);

    this->lua_ = nullptr;
}

void Editor::alloc_input(uv_handle_t* /* handle */, size_t /* recommendation */, uv_buf_t* buf) {
    // Large static input buffer to avoid memory allocation and frees.
    static std::array<char, 4096> input_buffer{};
    buf->base = input_buffer.data();
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
    std::size_t consumed{0};
    while (true) {
        if (auto [key, len] = Key::try_parse_ansi(self->input_buff_.data() + consumed); key) { // Successful parse.
            consumed += len;
            self->process_key(*key);
        } else if (self->input_buff_.size() == 1 && self->input_buff_[0] == '\x1b') { // Lone Esc.
            uv_timer_start(&self->esc_timer_, &Editor::esc_timer, 20, 0);
            break;
        } else {
            break;
        }
    }

    if (consumed > 0) { self->input_buff_.erase(0, consumed); }

    self->render();
}

void Editor::resize(uv_signal_t* handle, const int code) {
    auto* self = static_cast<Editor*>(handle->data);

    int width{};
    int height{};
    if (uv_tty_get_winsize(&self->tty_out_, &width, &height) != 0) { return; }

    self->display_.resize(width, height);

    if (const auto mb_height = self->mini_buffer_.viewport_->height_; std::cmp_greater(height, mb_height)) {
        height -= static_cast<int>(mb_height);
    }
    self->window_manager_.resize(width, height);
    self->mini_buffer_.viewport_->resize(width, 1, Position{.row_ = static_cast<std::size_t>(height), .col_ = 0});

    if (code != 0) { self->render(); }
}

void Editor::quit(uv_signal_t* handle, int /* code */) {
    // TODO: remove.
    (void)handle;
    // auto* self = static_cast<Editor*>(handle->data);
    // TODO: log info to use proper quit command.
}

void Editor::esc_timer(uv_timer_t* handle) {
    auto* self = static_cast<Editor*>(handle->data);
    self->process_key(Key{std::to_underlying(KeySpecial::ESCAPE), std::to_underlying(KeyMod::NONE)});
    // If this callback is called, input_buff_ only contains a single Esc key and can be safely cleared.
    self->input_buff_.clear();
    self->render();
}

void Editor::status_message_timer(uv_timer_t* handle) {
    auto* self = static_cast<Editor*>(handle->data);

    self->mini_buffer_.clear_status_message();

    self->render();
}

void Editor::set_status_message(std::string_view /* message */, bool /* force_viewport */) {
    /*
    // Stop existing timer.
    uv_timer_stop(&this->status_message_timer_);

    std::size_t tab_width = 4;
    if (const auto prop = this->mini_buffer_.viewport_->doc_->properties_["tab_width"]; prop.valid()) {
        tab_width = prop.get_or(4);
    }

    // 1. Fits in the Mini Buffer.
    if (!force_viewport) {
        if (const auto msg_width = utf8::str_width(message, 0, tab_width);
            msg_width < this->mini_buffer_.viewport_->width_ && message.find('\n') == std::string_view::npos) {
            this->mini_buffer_.set_status_message(message);

            uv_timer_start(&this->status_message_timer_, &Editor::status_message_timer, 5000, 0);

            this->render();
            return;
        }
    }

    // Lambda that searches for an existing status message window.
    auto exists = [&](this const auto& self, const std::shared_ptr<Window>& window) -> std::shared_ptr<Viewport> {
        if (!window) { return nullptr; }

        if (window->viewport_) { // Leaf node.
            sol::optional<std::string_view> minor_mode_override =
                window->viewport_->doc_->properties_["minor_mode_override"];
            if (minor_mode_override && *minor_mode_override == "status_message") { return window->viewport_; }

            return nullptr;
        }

        if (auto res = self(window->child_1_)) { return res; }
        return self(window->child_2_);
    };

    // 2. Reuse existing status message viewport.
    if (const auto viewport = exists(this->window_)) {
        this->active_viewport_ = viewport;
        viewport->doc_->clear();
        viewport->doc_->insert(0, message);

        viewport->move_cursor([](Cursor& c, const Document& d, std::size_t) -> void { c.point(d, 0); }, 0);

        this->render();
        return;
    }

    // 3. Create new split at the root.
    auto doc = this->create_document(std::nullopt);
    doc->insert(0, message);
    doc->properties_["minor_mode_override"] = "status_message";

    const auto new_viewport = this->create_viewport(1, 1, doc);

    const auto new_leaf = std::make_shared<Window>(new_viewport);
    this->window_ = std::make_shared<Window>(this->window_, new_leaf, true);
    this->window_->ratio_ = 0.75F;

    this->active_viewport_ = new_viewport;

    Editor::resize(&this->sigwinch_, 0);

    this->render();
    */
}

void Editor::enter_mini_buffer() {
    if (this->is_mini_buffer_) { return; }

    uv_timer_stop(&this->status_message_timer_);

    this->is_mini_buffer_ = true;
    this->mini_buffer_.prev_viewport_ = this->window_manager_.active_viewport_;
}

void Editor::exit_mini_buffer() {
    if (!this->is_mini_buffer_) { return; }

    ASSERT(this->mini_buffer_.prev_viewport_ != nullptr, "");

    this->is_mini_buffer_ = false;
    this->window_manager_.active_viewport_ = this->mini_buffer_.prev_viewport_;
    this->mini_buffer_.prev_viewport_ = nullptr;
}

void Editor::split_viewport(bool vertical, const float ratio) {
    if (!this->is_mini_buffer_) {
        this->window_manager_.split(vertical, ratio, this->create_viewport(this->window_manager_.active_viewport_));
    }
}

void Editor::resize_viewport(const float delta) {
    if (!this->is_mini_buffer_) { this->window_manager_.resize_split(delta); }
}

void Editor::close_viewport() {
    this->emit_event("viewport::destroyed", this->window_manager_.active_viewport_);

    // Stop event loop on last Viewport close.
    if (!this->window_manager_.close()) { uv_stop(this->loop_); }
}

void Editor::navigate_window(const Direction direction) {
    if (!this->is_mini_buffer_) { this->window_manager_.navigate(direction); }
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

    sol::protected_function resolve_face = (*this->lua_)["Core"]["Faces"]["resolve_face"];

    do {
        this->request_rendering_ = false;

        if (!this->window_manager_.render(this->display_, resolve_face)) { continue; }
        if (!this->mini_buffer_.viewport_->render(this->display_, resolve_face)) { continue; }
        this->window_manager_.active_viewport_->render_cursor(this->display_);
        this->display_.render(&this->tty_out_);
    } while (this->request_rendering_);

    this->is_rendering_ = false;
}

void Editor::process_key(const Key key) {
    if (auto on_input = (*this->lua_)["Core"]["Keybinds"]["on_input"]; !on_input.valid()) {
        // TODO: log error.
    } else if (const auto result = on_input(*this, key); !result.valid()) {
        const sol::error err = result;
        // TODO: log error.
    }
}
