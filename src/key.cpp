#include "key.hpp"

#include <charconv>

#include "typedef/key_mod.hpp"
#include "typedef/key_special.hpp"
#include "util/ansi.hpp"
#include "util/utf8.hpp"

auto Key::try_parse_ansi(std::string& buff) -> std::optional<Key> {
    if (buff.empty()) { return std::nullopt; }

    const auto ch = static_cast<unsigned char>(buff[0]);
    auto mods = static_cast<std::size_t>(KeyMod::NONE);

    // Ansi escape sequence.
    if (ch == 0x1B) {
        // Wait for more input. Editor handles lone Esc if no more input arrives.
        if (buff.size() == 1) { return std::nullopt; }

        if (buff[1] == '[') {
            // Wait for remaining part of sequence.
            if (buff.size() < 3) { return std::nullopt; }

            size_t end_idx = 2;
            // Find end of sequence which is either a letter or a tilde.
            for (; end_idx < buff.size() && isalpha(buff[end_idx]) == 0 && buff[end_idx] != '~'; end_idx += 1) {}
            if (end_idx == buff.size()) { return std::nullopt; }

            // Parse parameters between the ';'.
            std::vector<std::size_t> params{};
            size_t current = 2;
            while (current < end_idx) {
                std::size_t val = 0;
                if (auto [ptr, ec] = std::from_chars(buff.data() + current, buff.data() + end_idx, val);
                    ec == std::errc()) {
                    params.push_back(val);
                    current = ptr - buff.data();
                } else {
                    params.push_back(1);
                } // Parsing failed or empty param.

                // Skip separator ';'.
                if (current < end_idx && buff[current] == ';') {
                    current += 1;
                } else {
                    break;
                }
            }
            if (params.empty()) { params.push_back(1); }

            const auto suffix = static_cast<unsigned char>(buff[end_idx]);
            auto special_code = KeySpecial::NONE;
            if (suffix == '~') {
                // Parse modifier.
                mods |= ansi::parse_xterm_mod(params.size() > 1 ? params[1] : 1);

                // Parse key.
                switch (params[0]) {
                    case 2: {
                        special_code = KeySpecial::INSERT;
                        break;
                    }
                    case 3: {
                        special_code = KeySpecial::DELETE;
                        break;
                    }
                    default: break;
                }
            } else {
                // Parse modifier.
                mods |= ansi::parse_xterm_mod(params.size() > 1 ? params[1] : 1);

                // Parse key.
                switch (suffix) {
                    case 'A': {
                        special_code = KeySpecial::ARROW_UP;
                        break;
                    }
                    case 'B': {
                        special_code = KeySpecial::ARROW_DOWN;
                        break;
                    }
                    case 'C': {
                        special_code = KeySpecial::ARROW_RIGHT;
                        break;
                    }
                    case 'D': {
                        special_code = KeySpecial::ARROW_LEFT;
                        break;
                    }
                    default: break;
                }
            }

            if (special_code != KeySpecial::NONE) {
                buff.erase(0, end_idx + 1);
                return Key(static_cast<std::size_t>(special_code), mods);
            }
        } else if (buff[1] == 'O') { // Application mode.
            if (buff.size() < 3) { return std::nullopt; }

            auto special_code = KeySpecial::NONE;
            switch (buff[2]) {
                case 'A': {
                    special_code = KeySpecial::ARROW_UP;
                    break;
                }
                case 'B': {
                    special_code = KeySpecial::ARROW_DOWN;
                    break;
                }
                case 'C': {
                    special_code = KeySpecial::ARROW_RIGHT;
                    break;
                }
                case 'D': {
                    special_code = KeySpecial::ARROW_LEFT;
                    break;
                }
                default: break;
            }

            if (special_code != KeySpecial::NONE) {
                buff.erase(0, 3);
                return Key(static_cast<std::size_t>(special_code), mods);
            }
        }

        // Multiple Esc events, only process one.
        if (buff[1] == 0x1B) {
            buff.erase(0, 1);
            return Key(static_cast<std::size_t>(KeySpecial::ESCAPE), static_cast<std::size_t>(KeyMod::NONE));
        }

        // Alt + X.
        const auto next_char = static_cast<unsigned char>(buff[1]);
        const auto char_len = utf8::len(next_char);
        // Wait for full character.
        if (buff.size() < 1 + char_len) { return std::nullopt; }

        const auto code_point = utf8::decode({buff.data() + 1, char_len});
        buff.erase(0, 1 + char_len);
        return Key(code_point, static_cast<std::size_t>(KeyMod::ALT));
    }

    // Special ascii codes.
    if (ch == 127) { // Backspace.
        buff.erase(0, 1);
        return Key(static_cast<std::size_t>(KeySpecial::BACKSPACE), static_cast<std::size_t>(KeyMod::NONE));
    }
    if (ch < 32) { // Ctrl + X, ...
        buff.erase(0, 1);

        // Enter.
        if (ch == 13) {
            return Key(static_cast<std::size_t>(KeySpecial::ENTER), static_cast<std::size_t>(KeyMod::NONE));
        }
        // Tab.
        if (ch == 9) { return Key(static_cast<std::size_t>(KeySpecial::TAB), static_cast<std::size_t>(KeyMod::NONE)); }
        // Delete.
        if (ch == 8) {
            return Key(static_cast<std::size_t>(KeySpecial::BACKSPACE), static_cast<std::size_t>(KeyMod::NONE));
        }
        // Ctrl + X.
        return Key(ch + 'a' - 1, static_cast<std::size_t>(KeyMod::CTRL));
    }

    // UTF-8.
    const auto len = utf8::len(ch);
    // Wait for full character.
    if (buff.size() < len) { return std::nullopt; }

    const auto code_point = utf8::decode(buff);
    buff.erase(0, len);
    return Key(code_point, static_cast<std::size_t>(KeyMod::NONE));
}

auto Key::try_parse_string(const std::string_view buff, Key& out) -> bool {
    if (buff.empty()) { return false; }

    // Case 1: character literal.
    if (buff.front() != '<' || buff == "<" || buff == ">") {
        out = Key(utf8::decode(buff), static_cast<std::size_t>(KeyMod::NONE));
        return true;
    }
    // Case 2: bracketed sequence.
    if (buff.back() != '>') { return false; }

    // Strip brackets.
    const std::string_view content = buff.substr(1, buff.size() - 2);

    auto mods = static_cast<std::size_t>(KeyMod::NONE);
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
            if (const auto it = key::special_map.find(part); it != key::special_map.end()) {
                code = it->second;
            } else {
                code = utf8::decode(part);
            }
            break;
        }

        // Modifier.
        if (part == "C") {
            mods |= static_cast<std::size_t>(KeyMod::CTRL);
        } else if (part == "M") {
            mods |= static_cast<std::size_t>(KeyMod::ALT);
        } else if (part == "S") {
            mods |= static_cast<std::size_t>(KeyMod::SHIFT);
        } else {
            return false;
        }

        curr_pos = sep_pos + 1;
    }

    if (code == 0) { return false; }

    out = Key(code, mods);
    return true;
}

Key::Key(const std::size_t code, const std::size_t mod) : code_{code}, mod_{mod} {
    if ((mod & static_cast<std::size_t>(KeyMod::SHIFT)) != 0 && std::iswlower(static_cast<wint_t>(code)) != 0) {
        // Normalize and remove SHIFT flag.
        this->code_ = static_cast<std::size_t>(std::towupper(static_cast<wint_t>(code)));
        this->mod_ = static_cast<std::size_t>(mod) & ~static_cast<std::size_t>(KeyMod::SHIFT);
    }
}

auto Key::to_string() const -> std::string {
    if (this->code_ == 0) { return ""; }

    std::string ret{};

    // Special if it has a modifier, is not ASCII or is a space.
    const bool is_special = this->code_ >= 0x11000 || this->code_ == static_cast<std::size_t>(KeySpecial::BACKSPACE);
    const bool needs_brackets =
        this->mod_ != static_cast<std::size_t>(KeyMod::NONE) || is_special || this->code_ == ' ';

    if (needs_brackets) { ret += '<'; }

    if ((this->mod_ & static_cast<std::size_t>(KeyMod::CTRL)) != 0) { ret += "C-"; }
    if ((this->mod_ & static_cast<std::size_t>(KeyMod::ALT)) != 0) { ret += "M-"; }
    if ((this->mod_ & static_cast<std::size_t>(KeyMod::SHIFT)) != 0) { ret += "S-"; }

    if (is_special) { // Special keys.
        switch (static_cast<KeySpecial>(this->code_)) {
            case KeySpecial::BACKSPACE: {
                ret += "Bspc";
                break;
            }
            case KeySpecial::ARROW_UP: {
                ret += "Up";
                break;
            }
            case KeySpecial::ARROW_DOWN: {
                ret += "Down";
                break;
            }
            case KeySpecial::ARROW_LEFT: {
                ret += "Left";
                break;
            }
            case KeySpecial::ARROW_RIGHT: {
                ret += "Right";
                break;
            }
            case KeySpecial::ENTER: {
                ret += "Enter";
                break;
            }
            case KeySpecial::TAB: {
                ret += "Tab";
                break;
            }
            case KeySpecial::INSERT: {
                ret += "Ins";
                break;
            }
            case KeySpecial::DELETE: {
                ret += "Del";
                break;
            }
            case KeySpecial::ESCAPE: {
                ret += "Esc";
                break;
            }
            default: std::unreachable();
        }
    } else if (this->code_ == ' ') { // Space.
        ret += "Space";
    } else { // ASCII + Unicode.
        utf8::encode(ret, this->code_);
    }

    if (needs_brackets) { ret += '>'; }

    return ret;
}

auto Key::operator==(const Key& rhs) const -> bool { return this->code_ == rhs.code_ && this->mod_ == rhs.mod_; }
auto Key::operator!=(const Key& rhs) const -> bool { return !(*this == rhs); }

namespace key {
    std::unordered_map<std::string_view, std::size_t> special_map = {
        {"Enter", static_cast<std::size_t>(KeySpecial::ENTER)      },
        {"Tab",   static_cast<std::size_t>(KeySpecial::TAB)        },
        {"Space", ' '                                              },
        {"Bspc",  static_cast<std::size_t>(KeySpecial::BACKSPACE)  },
        {"Esc",   static_cast<std::size_t>(KeySpecial::ESCAPE)     },
        {"Up",    static_cast<std::size_t>(KeySpecial::ARROW_UP)   },
        {"Down",  static_cast<std::size_t>(KeySpecial::ARROW_DOWN) },
        {"Left",  static_cast<std::size_t>(KeySpecial::ARROW_LEFT) },
        {"Right", static_cast<std::size_t>(KeySpecial::ARROW_RIGHT)},
        {"Ins",   static_cast<std::size_t>(KeySpecial::INSERT)     },
        {"Del",   static_cast<std::size_t>(KeySpecial::DELETE)     },
        {"Lt",    '<'                                              },
        {"Gt",    '>'                                              },
    };
} // namespace key
