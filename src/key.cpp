#include "key.hpp"

#include <cctype>
#include <cwctype>
#include <string>
#include <utility>
#include <vector>

#include "util/ansi_parser.hpp"
#include "util/utf8.hpp"

auto parse_xterm_mod(const std::size_t param) -> std::size_t {
    auto mod = static_cast<std::size_t>(ModKey::NONE);
    const auto bitmap = param - 1;

    if ((bitmap & 1) != 0) { mod |= std::to_underlying(ModKey::SHIFT); }
    if ((bitmap & 2) != 0) { mod |= std::to_underlying(ModKey::ALT); }
    if ((bitmap & 4) != 0) { mod |= std::to_underlying(ModKey::CTRL); }
    if ((bitmap & 8) != 0) { mod |= std::to_underlying(ModKey::SUPER); }

    return mod;
}

auto Key::try_parse_ansi(const std::string_view buff) -> std::pair<std::optional<Key>, std::size_t> {
    if (buff.empty()) { return {std::nullopt, 0}; }

    AnsiParser parser{};
    std::optional<Key> key{std::nullopt};
    std::string utf8_ch{};

    parser.print_ = [&](uint8_t ch) -> void {
        utf8_ch.push_back(static_cast<char>(ch));

        auto expected_len = utf8::len(utf8_ch[0]);
        if (utf8_ch.size() >= expected_len) {
            auto code_point = utf8::decode(utf8_ch);
            key = Key(code_point, std::to_underlying(ModKey::NONE));
        }
    };

    parser.execute_ = [&](uint8_t ch) -> void {
        if (ch == 13) {
            key = Key(std::to_underlying(SpecialKey::ENTER), std::to_underlying(ModKey::NONE));
        } else if (ch == 9) {
            key = Key(std::to_underlying(SpecialKey::TAB), std::to_underlying(ModKey::NONE));
        } else if (ch == 8 || ch == 127) {
            key = Key(std::to_underlying(SpecialKey::BACKSPACE), std::to_underlying(ModKey::NONE));
        } else if (ch < 32) {
            key = Key(ch + 'a' - 1, std::to_underlying(ModKey::CTRL));
        }
    };

    parser.csi_dispatch_ = [&](const std::vector<int>& params, uint8_t ch, const std::string&) -> void {
        auto mods = std::to_underlying(ModKey::NONE);

        // Parse modifier.
        if (params.size() > 1 && params[1] > 1) { mods |= parse_xterm_mod(params[1]); }

        // Parse key.
        auto special_code = SpecialKey::NONE;
        if (ch == '~' && !params.empty()) {
            switch (params[0]) {
                case 2: special_code = SpecialKey::INSERT; break;
                case 3: special_code = SpecialKey::DELETE; break;
                default: break;
            }
        } else {
            // Parse key.
            switch (ch) {
                case 'A': special_code = SpecialKey::ARROW_UP; break;
                case 'B': special_code = SpecialKey::ARROW_DOWN; break;
                case 'C': special_code = SpecialKey::ARROW_RIGHT; break;
                case 'D': special_code = SpecialKey::ARROW_LEFT; break;
                case 'Z':
                    special_code = SpecialKey::TAB;
                    mods |= std::to_underlying(ModKey::SHIFT);
                    break;
                case 'u': { // Kitty protocol.
                    if (params.size() > 2 && params[2] != 0) {
                        key = Key(params[2], mods & ~std::to_underlying(ModKey::SHIFT));
                        return;
                    }
                    if (!params.empty()) {
                        switch (params[0]) {
                            case 8:
                            case 127: special_code = SpecialKey::BACKSPACE; break;
                            case 9: special_code = SpecialKey::TAB; break;
                            case 13: special_code = SpecialKey::ENTER; break;
                            case 27: special_code = SpecialKey::ESCAPE; break;
                            default: key = Key(params[0], mods); return;
                        }
                    }
                    break;
                }
                default: break;
            }
        }

        if (special_code != SpecialKey::NONE) { key = Key(std::to_underlying(special_code), mods); }
    };

    parser.esc_dispatch_ = [&](uint8_t ch, const std::string& inter) -> void {
        if (inter == "O") {
            auto special_code = SpecialKey::NONE;
            switch (ch) {
                case 'A': special_code = SpecialKey::ARROW_UP; break;
                case 'B': special_code = SpecialKey::ARROW_DOWN; break;
                case 'C': special_code = SpecialKey::ARROW_RIGHT; break;
                case 'D': special_code = SpecialKey::ARROW_LEFT; break;
                default: break;
            }
            if (special_code != SpecialKey::NONE) {
                key = Key(std::to_underlying(special_code), std::to_underlying(ModKey::NONE));
            }
        } else {
            key = Key(ch, std::to_underlying(ModKey::ALT));
        }
    };

    if (buff.size() == 1 && buff[0] == 0x1B) {
        return {Key(std::to_underlying(SpecialKey::ESCAPE), std::to_underlying(ModKey::NONE)), 1};
    }

    for (auto idx{0UZ}; idx < buff.size(); idx += 1) {
        parser.parse(buff[idx]);
        if (key.has_value()) { return {key, idx + 1}; }
    }

    return {std::nullopt, 0};
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
    // NOLINTBEGIN(bugprone-throwing-static-initialization)
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
    // NOLINTEND(bugprone-throwing-static-initialization)
} // namespace key
