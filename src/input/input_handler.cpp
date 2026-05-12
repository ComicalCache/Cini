#include "input_handler.hpp"

#include <utility>

#include "../util/utf8.hpp"

auto parse_xterm_mod(std::size_t param) -> std::size_t {
    auto mod{static_cast<std::size_t>(KeyEvent::ModKey::NONE)};
    const auto bitmap{param - 1};

    if ((bitmap & 1) != 0) { mod |= std::to_underlying(KeyEvent::ModKey::SHIFT); }
    if ((bitmap & 2) != 0) { mod |= std::to_underlying(KeyEvent::ModKey::ALT); }
    if ((bitmap & 4) != 0) { mod |= std::to_underlying(KeyEvent::ModKey::CTRL); }
    if ((bitmap & 8) != 0) { mod |= std::to_underlying(KeyEvent::ModKey::SUPER); }

    return mod;
}

InputHandler::InputHandler() {
    this->parser_.print_ = [this](const uint8_t ch) -> void {
        this->utf8_ch_.push_back(static_cast<char>(ch));

        const auto expected_len{utf8::len(this->utf8_ch_[0])};
        if (this->utf8_ch_.size() >= expected_len) {
            this->events_.emplace_back(
                KeyEvent{utf8::decode(this->utf8_ch_), std::to_underlying(KeyEvent::ModKey::NONE)});

            // Clear the parsed bytes.
            this->utf8_ch_.erase(0, expected_len);
        }
    };

    this->parser_.execute_ = [this](const uint8_t ch) -> void {
        if (ch == 13) {
            this->events_.emplace_back(
                KeyEvent{std::to_underlying(KeyEvent::SpecialKey::ENTER), std::to_underlying(KeyEvent::ModKey::NONE)});
        } else if (ch == 9) {
            this->events_.emplace_back(
                KeyEvent{std::to_underlying(KeyEvent::SpecialKey::TAB), std::to_underlying(KeyEvent::ModKey::NONE)});
        } else if (ch == 8 || ch == 127) {
            this->events_.emplace_back(
                KeyEvent{
                    std::to_underlying(KeyEvent::SpecialKey::BACKSPACE), std::to_underlying(KeyEvent::ModKey::NONE)});
        } else if (ch < 32) {
            this->events_.emplace_back(
                KeyEvent{static_cast<std::size_t>(ch + 'a' - 1), std::to_underlying(KeyEvent::ModKey::CTRL)});
        }
    };

    this->parser_.csi_dispatch_ =
        [this](const std::vector<int>& params, const uint8_t ch, const std::string& inter) -> void {
        // Mouse events.
        if (inter == "<" && (ch == 'M' || ch == 'm')) {
            if (params.size() < 3) { return; }

            const auto btn_info{params[0]};
            const auto x{static_cast<std::size_t>(params[1])};
            const auto y{static_cast<std::size_t>(params[2])};

            const auto btn_id{btn_info & 3};

            auto button{MouseEvent::MouseButton::NONE};
            switch (btn_id) {
                case 0: button = MouseEvent::MouseButton::LEFT; break;
                case 1: button = MouseEvent::MouseButton::MIDDLE; break;
                case 2: button = MouseEvent::MouseButton::RIGHT; break;
                default: break;
            }

            auto action{MouseEvent::MouseAction::PRESS};
            if (ch == 'm') {
                action = MouseEvent::MouseAction::RELEASE;
                button = MouseEvent::MouseButton::NONE;
            } else if ((btn_info & 64) != 0) {
                action = (btn_id == 0) ? MouseEvent::MouseAction::SCROLL_UP : MouseEvent::MouseAction::SCROLL_DOWN;
            } else if ((btn_info & 32) != 0) {
                action = MouseEvent::MouseAction::DRAG;
            }

            auto mod{std::to_underlying(KeyEvent::ModKey::NONE)};
            if (btn_info & 4) { mod |= std::to_underlying(KeyEvent::ModKey::SHIFT); }
            if (btn_info & 8) { mod |= std::to_underlying(KeyEvent::ModKey::ALT); }
            if (btn_info & 16) { mod |= std::to_underlying(KeyEvent::ModKey::CTRL); }

            this->events_.emplace_back(MouseEvent{.action_ = action, .button_ = button, .x_ = x, .y_ = y, .mod_ = mod});
            return;
        }

        auto mods{std::to_underlying(KeyEvent::ModKey::NONE)};

        // Parse modifier.
        if (params.size() > 1 && params[1] > 1) { mods |= parse_xterm_mod(params[1]); }

        // Parse key.
        auto special_code{KeyEvent::SpecialKey::NONE};
        if (ch == '~' && !params.empty()) {
            switch (params[0]) {
                case 2: special_code = KeyEvent::SpecialKey::INSERT; break;
                case 3: special_code = KeyEvent::SpecialKey::DELETE; break;
                default: break;
            }
        } else {
            // Parse key.
            switch (ch) {
                case 'A': special_code = KeyEvent::SpecialKey::ARROW_UP; break;
                case 'B': special_code = KeyEvent::SpecialKey::ARROW_DOWN; break;
                case 'C': special_code = KeyEvent::SpecialKey::ARROW_RIGHT; break;
                case 'D': special_code = KeyEvent::SpecialKey::ARROW_LEFT; break;
                case 'Z':
                    special_code = KeyEvent::SpecialKey::TAB;
                    mods |= std::to_underlying(KeyEvent::ModKey::SHIFT);
                    break;
                case 'u': { // Kitty protocol.
                    if (params.size() > 2 && params[2] != 0) {
                        this->events_.emplace_back(
                            KeyEvent{
                                static_cast<std::size_t>(params[2]),
                                mods & static_cast<std::size_t>(~std::to_underlying(KeyEvent::ModKey::SHIFT))});
                        return;
                    }
                    if (!params.empty()) {
                        switch (params[0]) {
                            case 8:
                            case 127: special_code = KeyEvent::SpecialKey::BACKSPACE; break;
                            case 9: special_code = KeyEvent::SpecialKey::TAB; break;
                            case 13: special_code = KeyEvent::SpecialKey::ENTER; break;
                            case 27: special_code = KeyEvent::SpecialKey::ESCAPE; break;
                            default:
                                this->events_.emplace_back(KeyEvent{static_cast<std::size_t>(params[0]), mods});
                                return;
                        }
                    }
                    break;
                }
                default: break;
            }
        }

        if (special_code != KeyEvent::SpecialKey::NONE) {
            this->events_.emplace_back(KeyEvent{std::to_underlying(special_code), mods});
        }
    };

    this->parser_.esc_dispatch_ = [this](const uint8_t ch, const std::string& inter) -> void {
        if (inter == "O") {
            auto special_code{KeyEvent::SpecialKey::NONE};
            switch (ch) {
                case 'A': special_code = KeyEvent::SpecialKey::ARROW_UP; break;
                case 'B': special_code = KeyEvent::SpecialKey::ARROW_DOWN; break;
                case 'C': special_code = KeyEvent::SpecialKey::ARROW_RIGHT; break;
                case 'D': special_code = KeyEvent::SpecialKey::ARROW_LEFT; break;
                default: break;
            }
            if (special_code != KeyEvent::SpecialKey::NONE) {
                this->events_.emplace_back(
                    KeyEvent{std::to_underlying(special_code), std::to_underlying(KeyEvent::ModKey::NONE)});
            }
        } else {
            this->events_.emplace_back(KeyEvent{ch, std::to_underlying(KeyEvent::ModKey::ALT)});
        }
    };
}

auto InputHandler::parse(const std::string_view data) -> std::vector<InputHandler::Event> {
    this->events_.clear();

    if (data.size() == 1 && data[0] == 0x1B) {
        this->events_.emplace_back(
            KeyEvent{std::to_underlying(KeyEvent::SpecialKey::ESCAPE), std::to_underlying(KeyEvent::ModKey::NONE)});
        return this->events_;
    }

    for (auto idx{0UZ}; idx < data.size(); idx += 1) { this->parser_.parse(data[idx]); }

    return this->events_;
}
