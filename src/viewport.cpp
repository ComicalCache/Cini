#include "viewport.hpp"

#include <cassert>
#include <charconv>
#include <utility>

#include "cell.hpp"
#include "editor.hpp"
#include "util.hpp"

void Viewport::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        "doc", &Viewport::doc_,
        "cursor", sol::property([](const Viewport& self) { return self.cursor(); }),
        "toggle_gutter", [](Viewport& self) { self.gutter_ = !self.gutter_; },
        "cursor_up", [](Viewport& self, const std::size_t n = 1) { self.move_cursor(&Cursor::up, n); },
        "cursor_down", [](Viewport& self, const std::size_t n = 1) { self.move_cursor(&Cursor::down, n); },
        "cursor_left", [](Viewport& self, const std::size_t n = 1) { self.move_cursor(&Cursor::left, n); },
        "cursor_right", [](Viewport& self, const std::size_t n = 1) { self.move_cursor(&Cursor::right, n); },
        "scroll_up", [](Viewport& self, const std::size_t n = 1) { self.scroll_up(n); },
        "scroll_down", [](Viewport& self, const std::size_t n = 1) { self.scroll_down(n); },
        "scroll_left", [](Viewport& self, const std::size_t n = 1) { self.scroll_left(n); },
        "scroll_right", [](Viewport& self, const std::size_t n   = 1) { self.scroll_right(n); });
    // clang-format on
}

Viewport::Viewport(const std::size_t width, const std::size_t height, std::shared_ptr<Document> doc)
    : doc_{std::move(doc)}, width_{width}, height_{height} { assert(this->doc_ != nullptr); }

Cursor Viewport::cursor() const { return this->cur_; }

void Viewport::move_cursor(const cursor::move_fn& move_fn, const std::size_t n) {
    move_fn(this->cur_, *this->doc_, n);
    this->adjust_viewport();
}

void Viewport::scroll_up(const std::size_t n) { this->scroll_.row_ = util::math::sub_sat(this->scroll_.row_, n); }

void Viewport::scroll_down(const std::size_t n) {
    if (const auto max_scroll = this->doc_->line_count(); this->scroll_.row_ + n < max_scroll) {
        this->scroll_.row_ += n;
    } else { this->scroll_.row_ = max_scroll; }
}

void Viewport::scroll_left(const std::size_t n) { this->scroll_.col_ = util::math::sub_sat(this->scroll_.col_, n); }

void Viewport::scroll_right(const std::size_t n) { this->scroll_.col_ += n; }

void Viewport::resize(const std::size_t width, const std::size_t height, const Position offset) {
    if (this->width_ == width && this->height_ == height) { return; }

    this->width_ = width;
    this->height_ = height;
    this->offset_ = offset;
}

void Viewport::render(Display& display, const Editor& editor) const {
    assert(this->doc_ != nullptr);
    if (this->width_ == 0 || this->height_ == 0) { return; }

    // Cache Faces and Replacements during rendering.
    face::FaceMap face_cache{};
    replacement::ReplacementMap replacement_cache{};

    auto face = [&](const std::string_view name) -> std::optional<Face> {
        if (const auto it = face_cache.find(name); it != face_cache.end()) { return it->second; }

        auto ret = editor.resolve_face(name, *this);
        if (ret) { face_cache.emplace(name, *ret); }
        return ret;
    };

    auto replacement = [&](const std::string_view name)-> std::optional<Replacement> {
        if (const auto it = replacement_cache.find(name); it != replacement_cache.end()) { return it->second; }

        auto ret = editor.resolve_replacement(name, *this);
        if (ret) { replacement_cache.emplace(name, *ret); }
        return ret;
    };

    const auto default_face = face("default");
    assert(default_face && default_face->fg_ && default_face->bg_);
    const auto gutter_face = face("gutter");
    assert(gutter_face && gutter_face->fg_ && gutter_face->bg_);

    std::size_t gutter_width = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter_width = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }
    auto content_width = util::math::sub_sat(this->width_, gutter_width);

    for (std::size_t y = 0; y < this->height_; y += 1) {
        const auto doc_y = this->scroll_.row_ + y;

        if (this->gutter_) {
            if (doc_y < this->doc_->line_count()) {
                // 32 characters for the line number should be plenty.
                char line_num[32]{};
                auto [ptr, ec] = std::to_chars(line_num, line_num + sizeof(line_num), doc_y + 1);
                std::size_t len = ptr - line_num;
                auto padding = gutter_width - 1 - len;

                Cell c(' ', *gutter_face->fg_, *gutter_face->bg_);

                // Draw padding.
                for (std::size_t x = 0; x < std::min(padding, this->width_); x += 1) {
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
                }
                // Draw number.
                for (std::size_t x = padding; x < std::min(padding + len, this->width_); x += 1) {
                    c.set_char(line_num[x - padding]);
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
                }
                // Draw trailing space.
                if (padding + len < this->width_) {
                    c.set_char(' ');
                    display.update(this->offset_.col_ + padding + len, this->offset_.row_ + y, c);
                }
            } else { // Draw empty gutter.
                Cell c(' ', *gutter_face->fg_, *gutter_face->bg_);
                for (std::size_t x = 0; x < std::min(gutter_width, this->width_); x += 1) {
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
                }
            }
        }

        if (doc_y < this->doc_->line_count()) {
            auto line = this->doc_->line(doc_y);
            const auto syntax_overlay = this->generated_syntax_overlay(editor, line);

            std::size_t x = 0;
            std::size_t idx = 0;
            while (idx < line.size()) {
                const auto len = util::utf8::len(line[idx]);
                // Draw the replacement character on malformed input.
                const auto ch = idx + len <= line.size() ? line.substr(idx, len) : "�";

                Cell cell("", *default_face->fg_, *default_face->bg_);

                // Layer 1: Syntax highlighting.
                if (syntax_overlay[idx]) {
                    if (const auto f = face(*syntax_overlay[idx]); f) {
                        if (f->fg_) { cell.fg_ = *f->fg_; }
                        if (f->bg_) { cell.bg_ = *f->bg_; }
                    }
                }

                // Layer 2: Character replacement.
                if (const auto r = replacement(ch); r) {
                    cell.set_utf8(r->txt);
                    if (const auto f = face(r->face); f) {
                        if (f->fg_) { cell.fg_ = *f->fg_; }
                        if (f->bg_) { cell.bg_ = *f->bg_; }
                    }
                } else { cell.set_utf8(ch); }

                const auto width = util::char_width(ch, x);
                if (x + width > this->scroll_.col_ && x < this->scroll_.col_ + content_width) {
                    for (std::size_t n = 0; n < width; n += 1) {
                        auto vx = x + n;

                        if (vx < this->scroll_.col_) continue;
                        if (vx >= this->scroll_.col_ + content_width) break;

                        vx = vx - this->scroll_.col_;

                        if (n == 0) { // Draw character.
                            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + y, cell);
                        } else { // Expand tab or wide characters.
                            Cell filler("", cell.fg_, cell.bg_);
                            if (ch == "\t") { // Tab.
                                filler.set_char(' ');
                            } else if (x < this->scroll_.col_) { // Half cutoff wide character.
                                filler.set_utf8("▯");
                            }

                            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + y, filler);
                        }
                    }
                }

                x += width;
                idx += len;
            }

            // Fill remainder of line.
            Cell c(' ', *default_face->fg_, *default_face->bg_);
            for (; x < this->scroll_.col_ + content_width; x += 1) {
                if (x >= this->scroll_.col_) {
                    const std::size_t screen_x = x - this->scroll_.col_ + gutter_width;
                    display.update(this->offset_.col_ + screen_x, this->offset_.row_ + y, c);
                }
            }
        } else { // Fill the empty line.
            Cell c(' ', *default_face->fg_, *default_face->bg_);
            for (std::size_t x = gutter_width; x < this->width_; x += 1) {
                display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
            }
        }
    }
}

void Viewport::render_cursor(Display& display) const {
    // Don't draw cursors outside the viewport.
    if (this->cur_.pos_.row_ < this->scroll_.row_ || this->cur_.pos_.row_ >= this->scroll_.row_ + this->height_) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }
    const auto y = this->cur_.pos_.row_ - this->scroll_.row_;

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    const auto line = this->doc_->line(y);

    std::size_t x = 0;
    std::size_t idx = 0;
    while (idx < line.size() && idx < this->cur_.pos_.col_) {
        const auto len = util::utf8::len(line[idx]);
        const auto width = util::char_width(line.substr(idx, len), x);

        x += width;
        idx += len;
    }

    // Don't draw cursors outside the viewport.
    if (x < this->scroll_.col_ || x >= this->scroll_.col_ + (this->width_ - gutter)) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }
    x = x - this->scroll_.col_;

    display.cursor(this->offset_.row_ + y, this->offset_.col_ + gutter + x);
}

void Viewport::adjust_viewport() {
    if (this->doc_->line_count() == 0) return;

    // 1. Vertical scrolling.
    if (this->cur_.pos_.row_ < this->scroll_.row_) { // Above.
        this->scroll_.row_ = this->cur_.pos_.row_;
    } else if (this->cur_.pos_.row_ >= this->scroll_.row_ + this->height_) { // Bellow.
        this->scroll_.row_ = this->cur_.pos_.row_ - this->height_ + 1;
    }

    if (this->cur_.pos_.row_ >= this->doc_->line_count()) return;

    const auto line = this->doc_->line(this->cur_.pos_.row_);

    // 2. Horizontal scrolling.
    std::size_t x = 0;
    std::size_t idx = 0;
    while (idx < line.size()) {
        const auto len = util::utf8::len(line[idx]);
        const auto width = util::char_width(line.substr(idx, len), x);

        if (x + width > this->cur_.pref_col_) { break; }

        x += width;
        idx += len;
    }

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    if (x < this->scroll_.col_) { // Left.
        this->scroll_.col_ = x;
    } else if (x >= this->scroll_.col_ + this->width_ - gutter) { // Right.
        this->scroll_.col_ = x - this->width_ - gutter + 1;
    }
}

std::vector<const std::string*> Viewport::generated_syntax_overlay(const Editor& editor,
                                                                   const std::string_view line) const {
    std::vector<const std::string*> overlay(line.size(), nullptr);

    auto apply_mode = [&](const Mode& mode) {
        for (const auto& [pattern, face]: mode.syntax_rules_) {
            const auto begin = std::regex_iterator(line.begin(), line.end(), pattern);
            std::regex_iterator<std::string_view::iterator> end{};

            for (auto match = begin; match != end; ++match) {
                const auto match_pos = std::distance(line.begin(), (*match)[0].first);

                if (const auto match_len = match->length();
                    match_pos + match_len <= static_cast<ptrdiff_t>(overlay.size())) {
                    std::fill_n(overlay.begin() + match_pos, match_len, &face);
                }
            }
        }
    };

    // 1. Major Mode.
    apply_mode(this->doc_->major_mode_);
    // 2. Global Minor Modes (evaluated front to back for precedence of later Minor Modes on the stack).
    for (const auto& mode: editor.get_global_minor_modes()) { apply_mode(mode); }
    // 3. Document Minor Modes (evaluated front to back for precedence of later Minor Modes on the stack).
    for (const auto& mode: this->doc_->minor_modes_) { apply_mode(mode); }

    return overlay;
}

