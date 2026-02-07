#include "key.hpp"

#include <charconv>
#include <cwctype>
#include <vector>

#include "types/mod_key.hpp"
#include "types/special_key.hpp"
#include "util/ansi.hpp"
#include "util/utf8.hpp"

auto Key::try_parse_ansi(const std::string_view buff) -> std::pair<std::optional<Key>, std::size_t> {
    if (buff.empty()) { return {std::nullopt, 0}; }

    const auto ch = static_cast<unsigned char>(buff[0]);
    auto mods = std::to_underlying(ModKey::NONE);

    // Ansi escape sequence.
    if (ch == 0x1B) {
        // Wait for more input. Editor handles lone Esc if no more input arrives.
        if (buff.size() == 1) { return {std::nullopt, 0}; }

        if (buff[1] == '[') {
            // Wait for remaining part of sequence.
            if (buff.size() < 3) { return {std::nullopt, 0}; }

            auto end_idx{2UZ};
            // Find end of sequence which is either a letter or a tilde.
            for (; end_idx < buff.size() && isalpha(buff[end_idx]) == 0 && buff[end_idx] != '~'; end_idx += 1) {}
            if (end_idx == buff.size()) { return {std::nullopt, 0}; }

            // Parse parameters between the ';'.
            std::vector<std::size_t> params{};
            auto current{2UZ};
            while (current < end_idx) {
                auto val{0UZ};
                if (auto [ptr, ec] = std::from_chars(buff.data() + current, buff.data() + end_idx, val);
                    ec == std::errc()) {
                    params.emplace_back(val);
                    current = ptr - buff.data();
                } else {
                    params.emplace_back(1);
                } // Parsing failed or empty param.

                // Skip separator ';'.
                if (current < end_idx && buff[current] == ';') {
                    current += 1;
                } else {
                    break;
                }
            }
            if (params.empty()) { params.emplace_back(1); }

            const auto suffix = static_cast<unsigned char>(buff[end_idx]);
            auto special_code = SpecialKey::NONE;
            if (suffix == '~') {
                // Parse modifier.
                mods |= ansi::parse_xterm_mod(params.size() > 1 ? params[1] : 1);

                // Parse key.
                switch (params[0]) {
                    case 2: special_code = SpecialKey::INSERT; break;
                    case 3: special_code = SpecialKey::DELETE; break;
                    default: break;
                }
            } else {
                // Parse modifier.
                mods |= ansi::parse_xterm_mod(params.size() > 1 ? params[1] : 1);

                // Parse key.
                switch (suffix) {
                    case 'A': special_code = SpecialKey::ARROW_UP; break;
                    case 'B': special_code = SpecialKey::ARROW_DOWN; break;
                    case 'C': special_code = SpecialKey::ARROW_RIGHT; break;
                    case 'D': special_code = SpecialKey::ARROW_LEFT; break;
                    case 'Z':
                        special_code = SpecialKey::TAB;
                        mods |= std::to_underlying(ModKey::SHIFT);
                        break;
                    case 'u': {
                        if (params.size() > 2 && params[2] != 0) {
                            return {Key(params[2], mods & ~std::to_underlying(ModKey::SHIFT)), end_idx + 1};
                        }

                        switch (params[0]) {
                            case 8: special_code = SpecialKey::BACKSPACE; break;
                            case 9: special_code = SpecialKey::TAB; break;
                            case 13: special_code = SpecialKey::ENTER; break;
                            case 27: special_code = SpecialKey::ESCAPE; break;
                            case 127: special_code = SpecialKey::BACKSPACE; break;
                            default: return {Key(params[0], mods), end_idx + 1};
                        }
                        break;
                    }
                    default: break;
                }
            }

            if (special_code != SpecialKey::NONE) { return {Key(std::to_underlying(special_code), mods), end_idx + 1}; }
        } else if (buff[1] == 'O') { // Application mode.
            if (buff.size() < 3) { return {std::nullopt, 0}; }

            auto special_code = SpecialKey::NONE;
            switch (buff[2]) {
                case 'A': special_code = SpecialKey::ARROW_UP; break;
                case 'B': special_code = SpecialKey::ARROW_DOWN; break;
                case 'C': special_code = SpecialKey::ARROW_RIGHT; break;
                case 'D': special_code = SpecialKey::ARROW_LEFT; break;
                default: break;
            }

            if (special_code != SpecialKey::NONE) { return {Key(std::to_underlying(special_code), mods), 3}; }
        }

        // Multiple Esc events, only process one.
        if (buff[1] == 0x1B) {
            return {Key(std::to_underlying(SpecialKey::ESCAPE), std::to_underlying(ModKey::NONE)), 1};
        }

        // Alt + X.
        const auto next_char = static_cast<unsigned char>(buff[1]);
        const auto char_len = utf8::len(next_char);
        // Wait for full character.
        if (buff.size() < 1 + char_len) { return {std::nullopt, 0}; }

        const auto code_point = utf8::decode({buff.data() + 1, char_len});
        return {Key(code_point, std::to_underlying(ModKey::ALT)), char_len + 1};
    }

    // Special ascii codes.
    // Backspace.
    if (ch == 127) { return {Key(std::to_underlying(SpecialKey::BACKSPACE), std::to_underlying(ModKey::NONE)), 1}; }
    if (ch < 32) { // Ctrl + X, ...
        // Enter.
        if (ch == 13) { return {Key(std::to_underlying(SpecialKey::ENTER), std::to_underlying(ModKey::NONE)), 1}; }
        // Tab.
        if (ch == 9) { return {Key(std::to_underlying(SpecialKey::TAB), std::to_underlying(ModKey::NONE)), 1}; }
        // Delete.
        if (ch == 8) { return {Key(std::to_underlying(SpecialKey::BACKSPACE), std::to_underlying(ModKey::NONE)), 1}; }
        // Ctrl + X.
        return {Key(ch + 'a' - 1, std::to_underlying(ModKey::CTRL)), 1};
    }

    // UTF-8.
    const auto len = utf8::len(ch);
    // Wait for full character.
    if (buff.size() < len) { return {std::nullopt, 0}; }

    const auto code_point = utf8::decode(buff);
    return {Key(code_point, std::to_underlying(ModKey::NONE)), len};
}

auto Key::try_parse_string(const std::string_view buff, Key& out) -> bool {
    if (buff.empty()) { return false; }

    // Case 1: character literal.
    if (buff.front() != '<' || buff == "<" || buff == ">") {
        out = Key(utf8::decode(buff), std::to_underlying(ModKey::NONE));
        return true;
    }
    // Case 2: bracketed sequence.
    if (buff.back() != '>') { return false; }

    // Strip brackets.
    const std::string_view content = buff.substr(1, buff.size() - 2);

    auto mods = std::to_underlying(ModKey::NONE);
    auto code{0UZ};

    auto curr_pos{0UZ};
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
            mods |= std::to_underlying(ModKey::CTRL);
        } else if (part == "M") {
            mods |= std::to_underlying(ModKey::ALT);
        } else if (part == "S") {
            mods |= std::to_underlying(ModKey::SHIFT);
        } else if (part == "P") {
            mods |= std::to_underlying(ModKey::SUPER);
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
    if ((mod & std::to_underlying(ModKey::SHIFT)) != 0 && std::iswlower(static_cast<wint_t>(code)) != 0) {
        // Normalize and remove SHIFT flag.
        this->code_ = static_cast<std::size_t>(std::towupper(static_cast<wint_t>(code)));
        this->mod_ = static_cast<std::size_t>(mod) & ~std::to_underlying(ModKey::SHIFT);
    }
}

auto Key::to_string() const -> std::string {
    if (this->code_ == 0) { return ""; }

    std::string ret{};

    // Special if it has a modifier, is not ASCII or is a backspace.
    const auto is_special = std::to_underlying(SpecialKey::ARROW_UP) <= this->code_
                         || this->code_ == std::to_underlying(SpecialKey::BACKSPACE);
    const auto needs_brackets = this->mod_ != std::to_underlying(ModKey::NONE) || is_special || this->code_ == ' ';

    if (needs_brackets) { ret += '<'; }

    if ((this->mod_ & std::to_underlying(ModKey::CTRL)) != 0) { ret += "C-"; }
    if ((this->mod_ & std::to_underlying(ModKey::ALT)) != 0) { ret += "M-"; }
    if ((this->mod_ & std::to_underlying(ModKey::SHIFT)) != 0) { ret += "S-"; }
    if ((this->mod_ & std::to_underlying(ModKey::SUPER)) != 0) { ret += "P-"; }

    if (is_special) { // Special keys.
        switch (static_cast<SpecialKey>(this->code_)) {
            case SpecialKey::BACKSPACE: ret += "Bspc"; break;
            case SpecialKey::ARROW_UP: ret += "Up"; break;
            case SpecialKey::ARROW_DOWN: ret += "Down"; break;
            case SpecialKey::ARROW_LEFT: ret += "Left"; break;
            case SpecialKey::ARROW_RIGHT: ret += "Right"; break;
            case SpecialKey::ENTER: ret += "Enter"; break;
            case SpecialKey::TAB: ret += "Tab"; break;
            case SpecialKey::INSERT: ret += "Ins"; break;
            case SpecialKey::DELETE: ret += "Del"; break;
            case SpecialKey::ESCAPE: ret += "Esc"; break;
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
        {"Enter", std::to_underlying(SpecialKey::ENTER)      },
        {"Tab",   std::to_underlying(SpecialKey::TAB)        },
        {"Space", ' '                                        },
        {"Bspc",  std::to_underlying(SpecialKey::BACKSPACE)  },
        {"Esc",   std::to_underlying(SpecialKey::ESCAPE)     },
        {"Up",    std::to_underlying(SpecialKey::ARROW_UP)   },
        {"Down",  std::to_underlying(SpecialKey::ARROW_DOWN) },
        {"Left",  std::to_underlying(SpecialKey::ARROW_LEFT) },
        {"Right", std::to_underlying(SpecialKey::ARROW_RIGHT)},
        {"Ins",   std::to_underlying(SpecialKey::INSERT)     },
        {"Del",   std::to_underlying(SpecialKey::DELETE)     },
        {"Lt",    '<'                                        },
        {"Gt",    '>'                                        },
    };
} // namespace key
