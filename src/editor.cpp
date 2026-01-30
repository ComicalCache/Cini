#include "editor.hpp"

#include "document.hpp"
#include "gen/lua_defaults.hpp"
#include "gen/version.hpp"
#include "key.hpp"
#include "regex.hpp"
#include "types/direction.hpp"
#include "types/face.hpp"
#include "types/key_mod.hpp"
#include "types/key_special.hpp"
#include "types/regex_match.hpp"
#include "util/fs.hpp"
#include "util/utf8.hpp"
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

    // This is only called when there is one Viewport left.
    self->emit_event("viewport::unfocused", self->workspace_.active_tree_viewport_);
    self->emit_event("document::unfocused", self->workspace_.active_tree_viewport_->doc_);
    self->emit_event("document::unloaded", self->workspace_.active_tree_viewport_->doc_);
    self->emit_event("document::destroyed", self->workspace_.active_tree_viewport_->doc_);
    self->emit_event("viewport::destroyed", self->workspace_.active_tree_viewport_);

    self->emit_event("cini::shutdown");
    self->shutdown();
    self->initialized_ = false;
}

auto Editor::instance() -> std::shared_ptr<Editor> {
    static std::shared_ptr<Editor> editor{nullptr};
    if (!editor) { editor = std::make_unique<Editor>(Editor::EditorKey{}); }

    return editor;
}

Editor::Editor(Editor::EditorKey /* key */) : workspace_{this->lua_}, loop_{uv_default_loop()} {}
Editor::~Editor() { this->shutdown(); }

auto Editor::create_document(std::optional<std::filesystem::path> path) -> std::shared_ptr<Document> {
    // Use existing Document if the backing file is identical.
    if (path) {
        fs::absolute(*path);

        for (const auto& doc: this->documents_) {
            if (!doc->path_) { continue; }

            auto same{false};
            if (std::filesystem::exists(*path) && std::filesystem::exists(*doc->path_)) {
                same = fs::equal(*path, *doc->path_);
            } else {
                same = *path == *doc->path_;
            }

            if (same) { return doc; }
        }
    }

    auto doc = this->documents_.emplace_back(std::make_shared<Document>(std::move(path), this->lua_));

    if (doc->path_) {
        this->emit_event("document::before-file-load", doc);
        if (auto content = fs::read_file(*doc->path_)) { doc->insert(0, *content); }
        this->emit_event("document::after-file-load", doc);
    }

    this->emit_event("document::created", doc);

    if (doc->path_ && !std::filesystem::is_directory(*doc->path_)) {
        if (const auto ext = doc->path_->extension().string(); ext.empty()) {
            this->emit_event("document::file-type", doc);
        } else {
            this->emit_event(std::format("document::file-type-{}", ext.substr(1)), doc);
        }
    }

    return doc;
}

void Editor::destroy_document(std::shared_ptr<Document> doc) {
    if (!doc) { return; }

    auto replacement = this->create_document(std::nullopt);

    // Switch all Viewport's Document that display this Document.
    this->workspace_.find_viewport([&](const std::shared_ptr<Viewport>& vp) -> bool {
        if (vp->doc_ == doc) { vp->change_document(replacement); }
        return false;
    });

    this->emit_event("document::destroyed", doc);

    std::erase(this->documents_, doc);
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

void Editor::set_status_message(std::string_view message, bool force_viewport) {
    auto tab_width{4UZ};
    if (const auto prop = this->workspace_.mini_buffer_.viewport_->doc_->properties_["tab_width"]; prop.valid()) {
        tab_width = prop.get_or(4);
    }

    // 1. Fits in the Mini Buffer.
    if (!force_viewport) {
        if (const auto msg_width = utf8::str_width(message, 0, tab_width);
            msg_width < this->workspace_.mini_buffer_.viewport_->width_
            && message.find('\n') == std::string_view::npos) {
            uv_timer_stop(&this->status_message_timer_);
            this->workspace_.mini_buffer_.set_status_message(message);
            uv_timer_start(&this->status_message_timer_, &Editor::status_message_timer, 5000, 0);

            this->render();
            return;
        }
    }

    auto status_viewport = this->workspace_.find_viewport([](const std::shared_ptr<Viewport>& vp) -> bool {
        if (!vp || !vp->doc_) { return false; }

        sol::optional<std::string_view> mode = vp->doc_->properties_["minor_mode_override"];
        return mode && *mode == "status_message";
    });

    // 2. Reuse existing status message viewport.
    if (status_viewport) {
        this->workspace_.active_tree_viewport_ = status_viewport;

        status_viewport->move_cursor([](Cursor& c, const Document& d, std::size_t) -> void { c.point(d, 0); }, 0);
        status_viewport->doc_->clear();
        status_viewport->doc_->insert(0, message);

        this->render();
        return;
    }

    // 3. Create new split at the root.
    auto doc = this->create_document(std::nullopt);
    doc->insert(0, message);
    doc->properties_["minor_mode_override"] = "status_message";

    this->workspace_.split_root(true, 0.75F, this->create_viewport(1, 1, doc));

    this->render();
}

void Editor::alloc_input(uv_handle_t* /* handle */, std::size_t /* recommendation */, uv_buf_t* buf) {
    // Large static input buffer to avoid memory allocation and frees.
    static std::array<char, 4096> input_buffer{};
    buf->base = input_buffer.data();
    buf->len = sizeof(input_buffer);
}

void Editor::input(uv_stream_t* stream, const ssize_t nread, const uv_buf_t* buf) {
    auto* self = static_cast<Editor*>(stream->data);

    if (nread < 0) {
        if (nread != UV_EOF) { self->set_status_message("Received invalid input event."); }
        return;
    }

    // Stop Esc waiting timers since new data arrived.
    uv_timer_stop(&self->esc_timer_);

    // Append new data to the input buffer. This handles partially received sequences.
    self->input_buff_.append(buf->base, nread);

    // Consume as many keys as possible.
    auto consumed{0UZ};
    while (true) {
        const auto view = std::string_view{self->input_buff_.data() + consumed, self->input_buff_.size() - consumed};
        if (auto [key, len] = Key::try_parse_ansi(view); key) { // Successful parse.
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

    if (const auto mb_height = self->workspace_.mini_buffer_.viewport_->height_; std::cmp_greater(height, mb_height)) {
        height -= static_cast<int>(mb_height);
    }
    self->workspace_.resize(width, height);
    self->workspace_.mini_buffer_.viewport_->resize(
        width, 1, Position{.row_ = static_cast<std::size_t>(height), .col_ = 0});

    if (code != 0) { self->render(); }
}

void Editor::quit(uv_signal_t* handle, int /* code */) {
    auto* self = static_cast<Editor*>(handle->data);
    self->set_status_message("Please use the quit command to exit.");
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
    self->workspace_.mini_buffer_.clear_status_message();
    self->render();
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
    this->lua_.open_libraries(
        sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::os, sol::lib::io, sol::lib::table,
        sol::lib::debug);

    // Handle Lua panic.
    this->lua_.set_panic([](lua_State* L) -> int {
        std::string s{};

        ansi::main_screen(s);
        ansi::disable_kitty_protocol(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << lua_tostring(L, -1) << "\n";

        uv_tty_reset_mode();
        exit(1);
    });
    // Generate trace on Lua errors.
    this->lua_.set_function("__panic", [](const sol::this_state L, const std::string_view& msg) -> std::string {
        return sol::state_view{L}["debug"]["traceback"](msg, 2);
    });
    sol::protected_function::set_default_handler(this->lua_["__panic"]);

    // Add a loader for predefined defaults.
    sol::table loaders = this->lua_["package"]["searchers"];
    loaders.add([](const sol::this_state L, const std::string_view& name) -> sol::optional<sol::function> {
        const auto it = lua_modules::files.find(name);
        if (it == lua_modules::files.end()) { return sol::nullopt; }

        const sol::load_result res = sol::state_view{L}.load(it->second, std::string{name});
        if (!res.valid()) { return sol::nullopt; }

        return res.get<sol::function>();
    });

    return *this;
}

auto Editor::init_bridge() -> Editor& {
    auto core = this->lua_.create_named_table("Core");

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
    this->lua_.new_usertype<State>("State",
        "editor", sol::property([this](const State&) -> std::reference_wrapper<Editor> { return std::ref(*this); }),
        "name", sol::property([](const State&) -> std::string_view { return version::NAME; }),
        "version", sol::property([](const State&) -> std::string_view { return version::VERSION; }),
        "build_date", sol::property([](const State&) -> std::string_view { return version::BUILD_DATE; }),
        "build_type", sol::property([](const State&) -> std::string_view { return version::BUILD_TYPE; }));
    this->lua_.set("State", State{});
    // clang-format on

    return *this;
}

auto Editor::init_state(const std::optional<std::filesystem::path>& path) -> Editor& {
    // Init lua with defaults.
    if (const auto result = this->lua_.safe_script("require('init')"); !result.valid()) {
        sol::error err = result;
        std::string s{};

        ansi::main_screen(s);
        ansi::disable_kitty_protocol(s);
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
        if (std::filesystem::exists(user_config)) {
            if (const auto result = this->lua_.safe_script_file(user_config); !result.valid()) {
                user_config_result = result;
            }
        }
    }

    this->workspace_.mini_buffer_ = MiniBuffer(1, 1, this->lua_);
    this->emit_event("mini_buffer::created", this->workspace_.mini_buffer_.viewport_);

    // One Document and Viewport must always exist.
    this->workspace_.set_root(this->create_viewport(1, 1, this->create_document(path)));

    // Initial render of the editor.
    // this->is_rendering_ is true to avoid errors during state initialization to be rendered before setup is completed.
    // Set it to false now.
    this->is_rendering_ = false;
    resize(&this->sigwinch_, 0);
    if (user_config_result) {
        this->set_status_message(std::format("Error in user config:\n{}", user_config_result->what()));
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

    this->lua_ = sol::state{};
}

void Editor::process_key(const Key key) {
    if (auto on_input = this->lua_["Core"]["Keybinds"]["on_input"]; !on_input.valid()) {
        std::string s{};

        ansi::main_screen(s);
        ansi::disable_kitty_protocol(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << "Failed to find 'Core.Keybinds.on_input'" << "\n";

        uv_tty_reset_mode();
        exit(1);
    } else if (const auto result = on_input(*this, key); !result.valid()) {
        const sol::error err = result;
        this->set_status_message(std::format("'Core.Keybinds.on_input' returned with error:\n{}", err.what()));
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

    sol::protected_function resolve_face = this->lua_["Core"]["Faces"]["resolve_face"];

    do {
        this->request_rendering_ = false;

        // Recalculate Main Windows/Mini Buffer split.
        const auto height = this->workspace_.height_ + this->workspace_.mini_buffer_.viewport_->height_;
        const auto mini_buffer_height =
            std::clamp(this->workspace_.mini_buffer_.viewport_->doc_->line_count(), 1UZ, height / 3);
        this->workspace_.resize(this->workspace_.width_, height - mini_buffer_height);
        this->workspace_.mini_buffer_.viewport_->resize(
            this->workspace_.mini_buffer_.viewport_->width_, mini_buffer_height,
            Position{.row_ = height - mini_buffer_height, .col_ = 0});

        if (!this->workspace_.render(this->display_, resolve_face)) { continue; }
        if (!this->workspace_.mini_buffer_.viewport_->render(this->display_, resolve_face)) { continue; }

        if (this->workspace_.is_mini_buffer_) {
            this->workspace_.mini_buffer_.viewport_->render_cursor(this->display_);
        } else {
            this->workspace_.active_tree_viewport_->render_cursor(this->display_);
        }

        this->display_.render(&this->tty_out_);
    } while (this->request_rendering_);

    this->is_rendering_ = false;
}
