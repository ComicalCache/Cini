#include "key_event.hpp"

#include <utility>

#include "../util/utf8.hpp"

auto KeyEvent::try_parse_string(const std::string_view buff, KeyEvent& out) -> bool {
    if (buff.empty()) { return false; }

    // Case 1: character literal.
    if (buff.front() != '<' || buff == "<" || buff == ">") {
        out = KeyEvent(utf8::decode(buff), std::to_underlying(KeyEvent::ModKey::NONE));
        return true;
    }
    // Case 2: bracketed sequence.
    if (buff.back() != '>') { return false; }

    // Strip brackets.
    const auto content{buff.substr(1, buff.size() - 2)};

    auto mods{std::to_underlying(KeyEvent::ModKey::NONE)};
    auto code{0UZ};

    auto curr_pos{0UZ};
    while (true) {
        const auto sep_pos{content.find('-', curr_pos)};
        auto end{sep_pos == std::string_view::npos};

        // The last character is a dash itself.
        if (!end && sep_pos == content.size() - 1) { end = true; }
        const auto part{end ? content.substr(curr_pos) : content.substr(curr_pos, sep_pos - curr_pos)};

        if (end) {
            // 1. Special key?
            if (const auto it{key_event::special_map.find(part)}; it != key_event::special_map.end()) {
                code = it->second;
            } else {
                code = utf8::decode(part);
            }
            break;
        }

        // Modifier.
        if (part == "C") {
            mods |= std::to_underlying(KeyEvent::ModKey::CTRL);
        } else if (part == "M") {
            mods |= std::to_underlying(KeyEvent::ModKey::ALT);
        } else if (part == "S") {
            mods |= std::to_underlying(KeyEvent::ModKey::SHIFT);
        } else if (part == "P") {
            mods |= std::to_underlying(KeyEvent::ModKey::SUPER);
        } else {
            return false;
        }

        curr_pos = sep_pos + 1;
    }

    if (code == 0) { return false; }

    out = KeyEvent(code, mods);
    return true;
}

KeyEvent::KeyEvent(const std::size_t code, const std::size_t mod) : code_{code}, mod_{mod} {
    if ((mod & std::to_underlying(KeyEvent::ModKey::SHIFT)) != 0 && std::iswlower(static_cast<wint_t>(code)) != 0) {
        // Normalize and remove SHIFT flag.
        this->code_ = static_cast<std::size_t>(std::towupper(static_cast<wint_t>(code)));
        this->mod_ = static_cast<std::size_t>(mod) & ~std::to_underlying(KeyEvent::ModKey::SHIFT);
    }
}

auto KeyEvent::to_string() const -> std::string {
    if (this->code_ == 0) { return ""; }

    std::string ret{};

    // Special if it has a modifier, is not ASCII or is a backspace.
    const auto is_special{
        std::to_underlying(KeyEvent::SpecialKey::ARROW_UP) <= this->code_
        || this->code_ == std::to_underlying(KeyEvent::SpecialKey::BACKSPACE)};
    const auto needs_brackets{
        this->mod_ != std::to_underlying(KeyEvent::ModKey::NONE) || is_special || this->code_ == ' '};

    if (needs_brackets) { ret += '<'; }

    if ((this->mod_ & std::to_underlying(KeyEvent::ModKey::CTRL)) != 0) { ret += "C-"; }
    if ((this->mod_ & std::to_underlying(KeyEvent::ModKey::ALT)) != 0) { ret += "M-"; }
    if ((this->mod_ & std::to_underlying(KeyEvent::ModKey::SHIFT)) != 0) { ret += "S-"; }
    if ((this->mod_ & std::to_underlying(KeyEvent::ModKey::SUPER)) != 0) { ret += "P-"; }

    if (is_special) { // Special keys.
        switch (static_cast<KeyEvent::SpecialKey>(this->code_)) {
            case KeyEvent::SpecialKey::BACKSPACE: ret += "Bspc"; break;
            case KeyEvent::SpecialKey::ARROW_UP: ret += "Up"; break;
            case KeyEvent::SpecialKey::ARROW_DOWN: ret += "Down"; break;
            case KeyEvent::SpecialKey::ARROW_LEFT: ret += "Left"; break;
            case KeyEvent::SpecialKey::ARROW_RIGHT: ret += "Right"; break;
            case KeyEvent::SpecialKey::ENTER: ret += "Enter"; break;
            case KeyEvent::SpecialKey::TAB: ret += "Tab"; break;
            case KeyEvent::SpecialKey::INSERT: ret += "Ins"; break;
            case KeyEvent::SpecialKey::DELETE: ret += "Del"; break;
            case KeyEvent::SpecialKey::ESCAPE: ret += "Esc"; break;
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

namespace key_event {
    // NOLINTBEGIN(bugprone-throwing-static-initialization)
    std::unordered_map<std::string_view, std::size_t> special_map = {
        {"Enter", std::to_underlying(KeyEvent::SpecialKey::ENTER)      },
        {"Tab",   std::to_underlying(KeyEvent::SpecialKey::TAB)        },
        {"Space", ' '                                                  },
        {"Bspc",  std::to_underlying(KeyEvent::SpecialKey::BACKSPACE)  },
        {"Esc",   std::to_underlying(KeyEvent::SpecialKey::ESCAPE)     },
        {"Up",    std::to_underlying(KeyEvent::SpecialKey::ARROW_UP)   },
        {"Down",  std::to_underlying(KeyEvent::SpecialKey::ARROW_DOWN) },
        {"Left",  std::to_underlying(KeyEvent::SpecialKey::ARROW_LEFT) },
        {"Right", std::to_underlying(KeyEvent::SpecialKey::ARROW_RIGHT)},
        {"Ins",   std::to_underlying(KeyEvent::SpecialKey::INSERT)     },
        {"Del",   std::to_underlying(KeyEvent::SpecialKey::DELETE)     },
        {"Lt",    '<'                                                  },
        {"Gt",    '>'                                                  },
    };
    // NOLINTEND(bugprone-throwing-static-initialization)
} // namespace key_event
