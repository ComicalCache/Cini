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

    // clang-format off
    std::unordered_map<std::string_view, std::size_t> special_map = {
        {"Enter", static_cast<std::size_t>(Special::ENTER)},
        {"Tab", static_cast<std::size_t>(Special::TAB)},
        {"Space", ' '},
        {"Bspc", static_cast<std::size_t>(Special::BACKSPACE)},
        {"Esc", static_cast<std::size_t>(Special::ESCAPE)},
        {"Up", static_cast<std::size_t>(Special::ARROW_UP)},
        {"Down", static_cast<std::size_t>(Special::ARROW_DOWN)},
        {"Left", static_cast<std::size_t>(Special::ARROW_LEFT)},
        {"Right", static_cast<std::size_t>(Special::ARROW_RIGHT)},
        {"Ins", static_cast<std::size_t>(Special::INSERT)},
        {"Del", static_cast<std::size_t>(Special::DELETE)},
        {"lt", '<'},
        {"gt", '>'},
    };
    // clang-format on
}

void Key::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Key>("Key",
        "to_string", &Key::to_string);
    // clang-format on
}

std::optional<Key> Key::try_parse_ansi(std::string& buff) {
    if (buff.empty()) { return std::nullopt; }

    const auto ch = static_cast<unsigned char>(buff[0]);
    auto mods = key::Mod::NONE;

    // Ansi escape sequence.
    if (ch == 0x1B) {
        // Wait for more input. Editor handles lone Esc if no more input arrives.
        if (buff.size() == 1) { return std::nullopt; }

        if (buff[1] == '[') {
            // Wait for remaining part of sequence.
            if (buff.size() < 3) { return std::nullopt; }

            size_t end_idx = 2;
            // Find end of sequence which is either a letter or a tilde.
            for (; end_idx < buff.size() && !isalpha(buff[end_idx]) && buff[end_idx] != '~'; end_idx += 1) {}
            if (end_idx == buff.size()) { return std::nullopt; }

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
                if (current < end_idx && buff[current] == ';') { current += 1; } else { break; }
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
                buff.erase(0, end_idx + 1);
                return Key(static_cast<std::size_t>(special_code), mods);
            }
        } else if (buff[1] == 'O') { // Application mode.
            if (buff.size() < 3) { return std::nullopt; }

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
                buff.erase(0, 3);
                return Key(static_cast<std::size_t>(special_code), mods);
            }
        }

        // Multiple Esc events, only process one.
        if (buff[1] == 0x1B) {
            buff.erase(0, 1);
            return Key(static_cast<std::size_t>(key::Special::ESCAPE), key::Mod::NONE);
        }

        // Alt + X.
        const auto next_char = static_cast<unsigned char>(buff[1]);
        const auto char_len = util::utf8::len(next_char);
        // Wait for full character.
        if (buff.size() < 1 + char_len) { return std::nullopt; }

        const auto code_point = util::utf8::decode({buff.data() + 1, char_len});
        buff.erase(0, 1 + char_len);
        return Key(code_point, key::Mod::ALT);
    }

    // Special ascii codes.
    if (ch == 127) { // Backspace.
        buff.erase(0, 1);
        return Key(static_cast<std::size_t>(key::Special::BACKSPACE), key::Mod::NONE);
    }
    if (ch < 32) { // Ctrl + X, ...
        buff.erase(0, 1);

        // Enter.
        if (ch == 13) { return Key(static_cast<std::size_t>(key::Special::ENTER), key::Mod::NONE); }
        // Tab.
        if (ch == 9) { return Key(static_cast<std::size_t>(key::Special::TAB), key::Mod::NONE); }
        // Delete.
        if (ch == 8) { return Key(static_cast<std::size_t>(key::Special::BACKSPACE), key::Mod::NONE); }
        // Ctrl + X.
        return Key(ch + 'a' - 1, key::Mod::CTRL);
    }

    // UTF-8.
    const auto len = util::utf8::len(ch);
    // Wait for full character.
    if (buff.size() < len) { return std::nullopt; }

    const auto code_point = util::utf8::decode(buff);
    buff.erase(0, len);
    return Key(code_point, key::Mod::NONE);
}

bool Key::try_parse_string(const std::string_view buff, Key& out) {
    if (buff.empty()) { return false; }

    // Case 1: character literal.
    if (buff.front() != '<' || buff == "<" || buff == ">") {
        out = Key(util::utf8::decode(buff), key::Mod::NONE);
        return true;
    }
    // Case 2: bracketed sequence.
    if (buff.back() != '>') { return false; }

    // Strip brackets.
    const std::string_view content = buff.substr(1, buff.size() - 2);

    auto mods = key::Mod::NONE;
    std::size_t code = 0;

    size_t curr_pos = 0;
    while (true) {
        const auto sep_pos = content.find('-', curr_pos);
        auto end = sep_pos == std::string_view::npos;

        // The last character is a dash itself.
        if (!end && sep_pos == content.size() - 1) { end = true; }
        const std::string_view part = end ? content.substr(curr_pos) : content.substr(curr_pos, sep_pos - curr_pos);

        if (end) {
            // 1. Special key?
            if (const auto it = key::special_map.find(part); it != key::special_map.end()) { code = it->second; }
            // 2. Character literal.
            else { code = util::utf8::decode(part); }
            break;
        }

        // Modifier.
        if (part == "C") { // Ctrl.
            mods |= key::Mod::CTRL;
        } else if (part == "M") { // Alt.
            mods |= key::Mod::ALT;
        } else if (part == "S") { // Shift.
            mods |= key::Mod::SHIFT;
        } else { return false; }

        curr_pos = sep_pos + 1;
    }

    if (code == 0) return false;

    out = Key(code, mods);
    return true;
}

Key::Key(const std::size_t code, const key::Mod mod)
    : code_{code}, mod_{mod} {
    if ((mod & key::Mod::SHIFT) == key::Mod::SHIFT && std::iswlower(static_cast<wint_t>(code))) {
        // Normalize and remove SHIFT flag.
        this->code_ = static_cast<std::size_t>(std::towupper(static_cast<wint_t>(code)));
        this->mod_ = static_cast<key::Mod>(static_cast<std::size_t>(mod) & ~static_cast<std::size_t>(key::Mod::SHIFT));
    }
}

std::string Key::to_string() const {
    if (this->code_ == 0) { return ""; }

    std::string ret{};

    // Special if it has a modifier, is not ASCII or is a space.
    const bool is_special = this->code_ >= 0x11000 || this->code_ == static_cast<std::size_t>(key::Special::BACKSPACE);
    const bool needs_brackets = this->mod_ != key::Mod::NONE || is_special || this->code_ == ' ';

    if (needs_brackets) { ret += '<'; }

    if ((this->mod_ & key::Mod::CTRL) == key::Mod::CTRL) { ret += "C-"; }
    if ((this->mod_ & key::Mod::ALT) == key::Mod::ALT) { ret += "M-"; }
    if ((this->mod_ & key::Mod::SHIFT) == key::Mod::SHIFT) { ret += "S-"; }

    if (is_special) { // Special keys.
        switch (static_cast<key::Special>(this->code_)) {
            case key::Special::BACKSPACE: {
                ret += "Bspc";
                break;
            }
            case key::Special::ARROW_UP: {
                ret += "Up";
                break;
            }
            case key::Special::ARROW_DOWN: {
                ret += "Down";
                break;
            }
            case key::Special::ARROW_LEFT: {
                ret += "Left";
                break;
            }
            case key::Special::ARROW_RIGHT: {
                ret += "Right";
                break;
            }
            case key::Special::ENTER: {
                ret += "Enter";
                break;
            }
            case key::Special::TAB: {
                ret += "Tab";
                break;
            }
            case key::Special::INSERT: {
                ret += "Ins";
                break;
            }
            case key::Special::DELETE: {
                ret += "Del";
                break;
            }
            case key::Special::ESCAPE: {
                ret += "Esc";
                break;
            }
            default: std::unreachable();
        }
    } else if (this->code_ == ' ') { // Space.
        ret += "Space";
    } else { // ASCII + Unicode.
        util::utf8::encode(ret, this->code_);
    }

    if (needs_brackets) { ret += '>'; }

    return ret;
}

bool Key::operator==(const Key& rhs) const { return this->code_ == rhs.code_ && this->mod_ == rhs.mod_; }

bool Key::operator!=(const Key& rhs) const { return !(*this == rhs); }

std::size_t std::hash<Key>::operator()(const Key& k) const noexcept {
    return std::hash<std::size_t>{}(k.code_) ^ std::hash<std::size_t>{}(static_cast<std::size_t>(k.mod_) << 1);
}
