#include "key.hpp"

#include <charconv>
#include <sstream>
#include <utility>
#include <vector>

#include "util.hpp"

namespace key {
    Mod parse_xterm_mod(const std::size_t param) {
        auto mod = Mod::NONE;
        const auto bitmap = param - 1;

        if (bitmap & 1) { mod |= Mod::SHIFT; }
        if (bitmap & 2) { mod |= Mod::ALT; }
        if (bitmap & 4) { mod |= Mod::CTRL; }

        return mod;
    }
}

Key::Key(const std::size_t code, const key::Mod mod) : code_{code}, mod_{mod} {}

std::string Key::to_string() const {
    if (this->code_ == 0) { return ""; }

    std::stringstream ss{};

    // Special if it has a modifier, is not ASCII or is a space.
    const bool is_special = this->code_ >= 0x11000 || this->code_ == static_cast<std::size_t>(key::Special::BACKSPACE);
    const bool needs_brackets = this->mod_ != key::Mod::NONE || is_special || this->code_ == ' ';

    if (needs_brackets) { ss << "<"; }

    if ((this->mod_ & key::Mod::CTRL) == key::Mod::CTRL) { ss << "C-"; }
    if ((this->mod_ & key::Mod::ALT) == key::Mod::ALT) { ss << "M-"; }
    if ((this->mod_ & key::Mod::SHIFT) == key::Mod::SHIFT) { ss << "S-"; }

    if (is_special) { // Special keys.
        switch (static_cast<key::Special>(this->code_)) {
            case key::Special::BACKSPACE: {
                ss << "Bspc";
                break;
            }
            case key::Special::ARROW_UP: {
                ss << "Up";
                break;
            }
            case key::Special::ARROW_DOWN: {
                ss << "Down";
                break;
            }
            case key::Special::ARROW_LEFT: {
                ss << "Left";
                break;
            }
            case key::Special::ARROW_RIGHT: {
                ss << "Right";
                break;
            }
            case key::Special::ENTER: {
                ss << "Enter";
                break;
            }
            case key::Special::TAB: {
                ss << "Tab";
                break;
            }
            case key::Special::INSERT: {
                ss << "Ins";
                break;
            }
            case key::Special::DELETE: {
                ss << "Del";
                break;
            }
            case key::Special::ESCAPE: {
                ss << "Esc";
                break;
            }
            default: std::unreachable();
        }
    } else if (this->code_ == ' ') { // Space.
        ss << "Space";
    } else { // ASCII + Unicode.
        util::utf8_encode(ss, this->code_);
    }

    if (needs_brackets) { ss << ">"; }

    return ss.str();
}

bool Key::operator==(const Key& rhs) const { return this->code_ == rhs.code_ && this->mod_ == rhs.mod_; }
bool Key::operator!=(const Key& rhs) const { return !(*this == rhs); }

bool Key::try_parse(std::string& buff, Key& out) {
    if (buff.empty()) { return false; }

    const auto ch = static_cast<unsigned char>(buff[0]);
    auto mods = key::Mod::NONE;

    // Ansi escape sequence.
    if (ch == 0x1B) {
        // Wait for more input. Editor handles lone Esc if no more input arrives.
        if (buff.size() == 1) { return false; }

        if (buff[1] == '[') {
            // Wait for remaining part of sequence.
            if (buff.size() < 3) { return false; }

            size_t end_idx = 2;
            // Find end of sequence which is either a letter or a tilde.
            for (; end_idx < buff.size() && !isalpha(buff[end_idx]) && buff[end_idx] != '~'; ++end_idx) {}
            if (end_idx == buff.size()) { return false; }

            // Parse parameters between the ';'.
            std::vector<std::size_t> params;
            size_t current = 2;
            while (current < end_idx) {
                std::size_t val = 0;
                if (auto [ptr, ec] = std::from_chars(buff.data() + current, buff.data() + end_idx, val);
                    ec == std::errc()) {
                    params.push_back(val);
                    current = ptr - buff.data();
                } else { params.push_back(1); } // Parsing failed or empty param.

                // Skip separator ';'.
                if (current < end_idx && buff[current] == ';') { current++; } else { break; }
            }
            if (params.empty()) { params.push_back(1); }

            const auto suffix = static_cast<unsigned char>(buff[end_idx]);
            auto special_code = key::Special::NONE;
            if (suffix == '~') {
                // Parse modifier.
                mods |= key::parse_xterm_mod(params.size() > 1 ? params[1] : 1);

                // Parse key.
                switch (params[0]) {
                    case 2: {
                        special_code = key::Special::INSERT;
                        break;
                    }
                    case 3: {
                        special_code = key::Special::DELETE;
                        break;
                    }
                    default: break;
                }
            } else {
                // Parse modifier.
                mods |= key::parse_xterm_mod(params.size() > 1 ? params[1] : 1);

                // Parse key.
                switch (suffix) {
                    case 'A': {
                        special_code = key::Special::ARROW_UP;
                        break;
                    }
                    case 'B': {
                        special_code = key::Special::ARROW_DOWN;
                        break;
                    }
                    case 'C': {
                        special_code = key::Special::ARROW_RIGHT;
                        break;
                    }
                    case 'D': {
                        special_code = key::Special::ARROW_LEFT;
                        break;
                    }
                    default: break;
                }
            }

            if (special_code != key::Special::NONE) {
                out = Key(static_cast<std::size_t>(special_code), mods);
                buff.erase(0, end_idx + 1);
                return true;
            }
        } else if (buff[1] == 'O') { // Application mode.
            if (buff.size() < 3) { return false; }

            auto special_code = key::Special::NONE;
            switch (buff[2]) {
                case 'A': {
                    special_code = key::Special::ARROW_UP;
                    break;
                }
                case 'B': {
                    special_code = key::Special::ARROW_DOWN;
                    break;
                }
                case 'C': {
                    special_code = key::Special::ARROW_RIGHT;
                    break;
                }
                case 'D': {
                    special_code = key::Special::ARROW_LEFT;
                    break;
                }
                default: break;
            }

            if (special_code != key::Special::NONE) {
                out = Key(static_cast<std::size_t>(special_code), mods);
                buff.erase(0, 3);
                return true;
            }
        }

        // Multiple Esc events, only process one.
        if (buff[1] == 0x1B) {
            out = Key(static_cast<std::size_t>(key::Special::ESCAPE), key::Mod::NONE);
            buff.erase(0, 1);
            return true;
        }

        // Alt + X.
        const auto next_char = static_cast<unsigned char>(buff[1]);
        const auto char_len = util::utf8_len(next_char);
        // Wait for full character.
        if (buff.size() < 1 + char_len) { return false; }
        out = Key(util::utf8_decode({buff.data() + 1, char_len}), key::Mod::ALT);

        buff.erase(0, 1 + char_len);
        return true;
    }

    // Special ascii codes.
    if (ch == 127) { // Backspace.
        out = Key(static_cast<std::size_t>(key::Special::BACKSPACE), key::Mod::NONE);
        buff.erase(0, 1);
        return true;
    }
    if (ch < 32) { // Ctrl + X, ...
        // Enter.
        if (ch == 13) { out = Key(static_cast<std::size_t>(key::Special::ENTER), key::Mod::NONE); }
        // Tab.
        else if (ch == 9) { out = Key(static_cast<std::size_t>(key::Special::TAB), key::Mod::NONE); }
        // Delete.
        else if (ch == 8) { out = Key(static_cast<std::size_t>(key::Special::BACKSPACE), key::Mod::NONE); }
        // Ctrl + X.
        else { out = Key(ch + 'a' - 1, key::Mod::CTRL); }

        buff.erase(0, 1);
        return true;
    }

    // UTF-8.
    const auto len = util::utf8_len(ch);
    // Wait for full character.
    if (buff.size() < len) { return false; }
    out = Key(util::utf8_decode(buff), key::Mod::NONE);

    buff.erase(0, len);
    return true;
}
