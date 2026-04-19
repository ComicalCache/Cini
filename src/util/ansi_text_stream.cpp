#include "ansi_text_stream.hpp"

#include <charconv>

#include <sol/object.hpp>

#include "../document.hpp"
#include "../editor.hpp"
#include "../types/face.hpp"

AnsiTextStream::AnsiTextStream(std::shared_ptr<Document> doc) : doc_{std::move(doc)} {}

auto AnsiTextStream::parse(const std::string_view text, std::size_t pos) -> std::size_t {
    auto raw_chunk{this->buffer_ + std::string{text}};
    this->buffer_.clear();

    std::string chunk;
    chunk.reserve(raw_chunk.size());
    for (auto idx{0UZ}; idx < raw_chunk.size(); idx += 1) {
        if (raw_chunk[idx] == '\r') {
            if (idx + 1 == raw_chunk.size()) { // '\r' is the last byte of this chunk.
                this->buffer_ = "\r";
                break;
            }
            if (raw_chunk[idx + 1] == '\n') { // CRLF.
                continue;
            }

            // Convert \r to \n to handle things like progress bars gracefully.
            chunk.push_back('\n');
        } else {
            chunk.push_back(raw_chunk[idx]);
        }
    }

    if (chunk.empty()) { return pos; }

    auto editor = Editor::instance();
    auto apply_styles = [&](const std::size_t start, const std::size_t stop) -> void {
        if (start == stop) { return; }

        if (this->fg_ != sol::lua_nil) { this->doc_->add_text_property(start, stop, "ansi.fg", this->fg_); }
        if (this->bg_ != sol::lua_nil) { this->doc_->add_text_property(start, stop, "ansi.bg", this->bg_); }
        if (this->style_ != sol::lua_nil) { this->doc_->add_text_property(start, stop, "ansi.style", this->style_); }
    };

    auto idx{0UZ};
    while (idx < chunk.size()) {
        auto esc_pos{chunk.find("\x1b[", idx)};

        if (esc_pos == std::string::npos) {
            // Check for a partial escape sequence.
            if (auto partial_esc{chunk.find('\x1b', idx)}; partial_esc != std::string::npos) {
                if (auto text{chunk.substr(idx, partial_esc - idx)}; !text.empty()) {
                    this->doc_->insert(pos, text);
                    apply_styles(pos, pos + text.size());

                    pos += text.size();
                }

                this->buffer_ = chunk.substr(partial_esc);
                break;
            }

            // Insert remaining plain text.
            auto plain_text{chunk.substr(idx)};
            this->doc_->insert(pos, plain_text);
            apply_styles(pos, pos + plain_text.size());

            pos += plain_text.size();
            break;
        }

        // Insert text before the escape sequence.
        if (esc_pos > idx) {
            auto text{chunk.substr(idx, esc_pos - idx)};
            this->doc_->insert(pos, text);
            apply_styles(pos, pos + text.size());

            pos += text.size();
        }

        // Find the sequence terminator.
        auto term_pos{esc_pos + 2};
        while (term_pos < chunk.size() && chunk[term_pos] >= 0x20 && chunk[term_pos] <= 0x3F) { term_pos += 1; }

        if (term_pos == chunk.size()) {
            // Incomplete sequence.
            this->buffer_ = chunk.substr(esc_pos);
            break;
        }

        // SGR sequence.
        if (chunk[term_pos] == 'm') {
            auto codes{chunk.substr(esc_pos + 2, term_pos - (esc_pos + 2))};
            process_sgr(codes);
        }

        // Advance past the terminator.
        idx = term_pos + 1;
    }

    return pos;
}

void AnsiTextStream::process_sgr(const std::string_view sgr_codes) {
    // `\x1b[m` is equivalent to reset \x1b[0m.
    if (sgr_codes.empty()) {
        this->fg_ = sol::object{sol::lua_nil};
        this->bg_ = sol::object{sol::lua_nil};
        this->style_mask_ = std::to_underlying(StyleMask::NONE);
        this->style_ = sol::object{sol::lua_nil};

        return;
    }

    std::vector<std::size_t> codes{};
    auto start{0UZ};
    while (start < sgr_codes.size()) {
        auto end{sgr_codes.find(';', start)};
        auto code_str{sgr_codes.substr(start, end - start)};

        auto code{0UZ};
        if (!code_str.empty()) { std::from_chars(code_str.data(), code_str.data() + code_str.size(), code); }
        codes.push_back(code);

        if (end == std::string_view::npos) { break; }
        start = end + 1;
    }

    for (auto idx{0UZ}; idx < codes.size(); idx += 1) {
        auto code = codes[idx];

        switch (code) {
            // Reset.
            case 0:
                this->fg_ = sol::object{sol::lua_nil};
                this->bg_ = sol::object{sol::lua_nil};
                this->style_mask_ = std::to_underlying(StyleMask::NONE);
                this->style_ = sol::object{sol::lua_nil};
                break;

            // Set style.
            case 1:
                this->style_mask_ |= std::to_underlying(StyleMask::BOLD);
                this->style_ = get_style();
                break;
            case 3:
                this->style_mask_ |= std::to_underlying(StyleMask::ITALIC);
                this->style_ = get_style();
                break;
            case 4:
                this->style_mask_ |= std::to_underlying(StyleMask::UNDERLINE);
                this->style_ = get_style();
                break;
            case 9:
                this->style_mask_ |= std::to_underlying(StyleMask::STRIKETHROUGH);
                this->style_ = get_style();
                break;

            // Reset style.
            case 22:
                this->style_mask_ &= ~std::to_underlying(StyleMask::BOLD);
                this->style_ = get_style();
                break;
            case 23:
                this->style_mask_ &= ~std::to_underlying(StyleMask::ITALIC);
                this->style_ = get_style();
                break;
            case 24:
                this->style_mask_ &= ~std::to_underlying(StyleMask::UNDERLINE);
                this->style_ = get_style();
                break;
            case 29:
                this->style_mask_ &= ~std::to_underlying(StyleMask::STRIKETHROUGH);
                this->style_ = get_style();
                break;

            // Reset colors.
            case 39: this->fg_ = sol::object{sol::lua_nil}; break;
            case 49: this->bg_ = sol::object{sol::lua_nil}; break;

            // True colors.
            case 38:
            case 48:
                if (idx + 4 < codes.size() && codes[idx + 1] == 2) { // RGB format: 38;2;R;G;B or 48;2;R;G;B.
                    auto r{static_cast<uint8_t>(codes[idx + 2])};
                    auto g{static_cast<uint8_t>(codes[idx + 3])};
                    auto b{static_cast<uint8_t>(codes[idx + 4])};

                    if (code == 38) {
                        this->fg_ = get_rgb_fg(r, g, b);
                    } else {
                        this->bg_ = get_rgb_bg(r, g, b);
                    }

                    idx += 4;
                } else if (idx + 2 < codes.size() && codes[idx + 1] == 5) { // 8-bit format: 38;5;n or 48;5;n.
                    auto color = codes[idx + 2];

                    if (color >= 0 && color <= 15) { // Map to standard 16 colors.
                        std::size_t mapped_code;
                        if (color < 8) {
                            mapped_code = (code == 38) ? (30 + color) : (40 + color);
                        } else {
                            mapped_code = (code == 38) ? (90 + color - 8) : (100 + color - 8);
                        }

                        if (code == 38) {
                            this->fg_ = get_fg(mapped_code);
                        } else {
                            this->bg_ = get_bg(mapped_code);
                        }

                    } else if (color >= 16 && color <= 231) { // 6x6x6 color cube.
                        auto n{color - 16};
                        uint8_t r = (n / 36) > 0 ? 55 + ((n / 36) * 40) : 0;
                        uint8_t g = ((n / 6) % 6) > 0 ? 55 + (((n / 6) % 6) * 40) : 0;
                        uint8_t b = (n % 6) > 0 ? 55 + ((n % 6) * 40) : 0;

                        if (code == 38) {
                            this->fg_ = get_rgb_fg(r, g, b);
                        } else {
                            this->bg_ = get_rgb_bg(r, g, b);
                        }

                    } else if (color >= 232 && color <= 255) { // 24-step grayscale.
                        uint8_t v = ((color - 232) * 10) + 8;

                        if (code == 38) {
                            this->fg_ = get_rgb_fg(v, v, v);
                        } else {
                            this->bg_ = get_rgb_bg(v, v, v);
                        }
                    }

                    idx += 2;
                }
                break;

            // Standard foreground and bright foreground.
            case 30 ... 37:
            case 90 ... 97: this->fg_ = get_fg(code); break;

            // Standard background and bright background.
            case 40 ... 47:
            case 100 ... 107: this->bg_ = get_bg(code); break;

            default: break;
        }
    }
}

auto AnsiTextStream::get_fg(const std::size_t code) -> sol::object {
    if (const auto it{this->fg_cache_.find(code)}; it != this->fg_cache_.end()) { return it->second; }

    std::string face{};
    switch (code) {
        // Standard foreground.
        case 30: face = "ansi.fg.black"; break;
        case 31: face = "ansi.fg.red"; break;
        case 32: face = "ansi.fg.green"; break;
        case 33: face = "ansi.fg.yellow"; break;
        case 34: face = "ansi.fg.blue"; break;
        case 35: face = "ansi.fg.magenta"; break;
        case 36: face = "ansi.fg.cyan"; break;
        case 37: face = "ansi.fg.white"; break;

        // Bright foreground.
        case 90: face = "ansi.fg.bright_black"; break;
        case 91: face = "ansi.fg.bright_red"; break;
        case 92: face = "ansi.fg.bright_green"; break;
        case 93: face = "ansi.fg.bright_yellow"; break;
        case 94: face = "ansi.fg.bright_blue"; break;
        case 95: face = "ansi.fg.bright_magenta"; break;
        case 96: face = "ansi.fg.bright_cyan"; break;
        case 97: face = "ansi.fg.bright_white"; break;

        default: return sol::lua_nil;
    }

    auto obj{sol::make_object(Editor::instance()->lua_, face)};
    this->fg_cache_[code] = obj;
    return obj;
}

auto AnsiTextStream::get_bg(const std::size_t code) -> sol::object {
    if (const auto it{this->bg_cache_.find(code)}; it != this->bg_cache_.end()) { return it->second; }

    std::string face;
    switch (code) {
        // Standard background.
        case 40: face = "ansi.bg.black"; break;
        case 41: face = "ansi.bg.red"; break;
        case 42: face = "ansi.bg.green"; break;
        case 43: face = "ansi.bg.yellow"; break;
        case 44: face = "ansi.bg.blue"; break;
        case 45: face = "ansi.bg.magenta"; break;
        case 46: face = "ansi.bg.cyan"; break;
        case 47: face = "ansi.bg.white"; break;

        // Bright background.
        case 100: face = "ansi.bg.bright_black"; break;
        case 101: face = "ansi.bg.bright_red"; break;
        case 102: face = "ansi.bg.bright_green"; break;
        case 103: face = "ansi.bg.bright_yellow"; break;
        case 104: face = "ansi.bg.bright_blue"; break;
        case 105: face = "ansi.bg.bright_magenta"; break;
        case 106: face = "ansi.bg.bright_cyan"; break;
        case 107: face = "ansi.bg.bright_white"; break;

        default: return sol::lua_nil;
    }

    auto obj{sol::make_object(Editor::instance()->lua_, face)};
    this->bg_cache_[code] = obj;
    return obj;
}

auto AnsiTextStream::get_style() -> sol::object {
    if (this->style_mask_ == std::to_underlying(StyleMask::NONE)) { return sol::lua_nil; }

    if (this->style_cache_[this->style_mask_] == sol::lua_nil) {
        Face f{};

        if ((this->style_mask_ & std::to_underlying(StyleMask::BOLD)) != 0) { f.bold_ = true; }
        if ((this->style_mask_ & std::to_underlying(StyleMask::ITALIC)) != 0) { f.italic_ = true; }
        if ((this->style_mask_ & std::to_underlying(StyleMask::UNDERLINE)) != 0) { f.underline_ = true; }
        if ((this->style_mask_ & std::to_underlying(StyleMask::STRIKETHROUGH)) != 0) { f.strikethrough_ = true; }

        this->style_cache_[this->style_mask_] = sol::make_object(Editor::instance()->lua_, f);
    }
    return this->style_cache_[this->style_mask_];
}

auto AnsiTextStream::get_rgb_fg(uint8_t r, uint8_t g, uint8_t b) -> sol::object {
    auto key{(static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b};

    if (const auto it{this->rgb_fg_cache_.find(key)}; it != this->rgb_fg_cache_.end()) { return it->second; }

    Face face{};
    face.fg_ = Rgb{.r_ = r, .g_ = g, .b_ = b};

    auto obj{sol::make_object(Editor::instance()->lua_, face)};
    this->rgb_fg_cache_[key] = obj;
    return obj;
}

auto AnsiTextStream::get_rgb_bg(uint8_t r, uint8_t g, uint8_t b) -> sol::object {
    uint32_t key = (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b;

    if (const auto it = this->rgb_bg_cache_.find(key); it != this->rgb_bg_cache_.end()) { return it->second; }

    Face face{};
    face.bg_ = Rgb{.r_ = r, .g_ = g, .b_ = b};

    auto obj{sol::make_object(Editor::instance()->lua_, face)};
    this->rgb_bg_cache_[key] = obj;
    return obj;
}
