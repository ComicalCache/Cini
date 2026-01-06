#include "viewport.hpp"

#include "document.hpp"
#include "editor.hpp"
#include "mode.hpp"
#include "util.hpp"

void Viewport::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Viewport>("Viewport",
        // Properties.
        "doc", &Viewport::doc_,
        "cursor", sol::property([](Viewport& self) { return &self.cur_; }),

        // Functions.
        "move_cursor", &Viewport::move_cursor,
        "toggle_gutter", [](Viewport& self) {
            self.gutter_ = !self.gutter_;
            self.adjust_viewport();
        },
        "set_mode_line", [](Viewport& self, const sol::function& callback) { self.mode_line_renderer_ = callback; },
        "toggle_mode_line", [](Viewport& self) {
            self.mode_line_ = !self.mode_line_;
            self.adjust_viewport();
        },
        "scroll_up", [](Viewport& self, const std::size_t n = 1) { self.scroll_up(n); },
        "scroll_down", [](Viewport& self, const std::size_t n = 1) { self.scroll_down(n); },
        "scroll_left", [](Viewport& self, const std::size_t n = 1) { self.scroll_left(n); },
        "scroll_right", [](Viewport& self, const std::size_t n   = 1) { self.scroll_right(n); });
    // clang-format on
}

Viewport::Viewport(const std::size_t width, const std::size_t height, std::shared_ptr<Document> doc)
    : doc_{std::move(doc)}, width_{width}, height_{height} { assert(this->doc_ != nullptr); }

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
    if (this->width_ == width && this->height_ == height // Dimensions.
        && this->offset_.row_ == offset.row_ && this->offset_.col_ == offset.col_) // Offset.
    {
        return;
    }

    this->width_ = width;
    this->height_ = height;
    this->offset_ = offset;

    this->adjust_viewport();
}

void Viewport::render(Display& display, const Editor& editor) const {
    assert(this->doc_ != nullptr);

    // TODO: log errors.
    auto height = this->mode_line_ && this->mode_line_renderer_.valid()
                      ? util::math::sub_sat(this->height_, static_cast<std::size_t>(1))
                      : this->height_;
    if (this->width_ == 0 || height == 0) { return; }

    // Cache Faces and Replacements during rendering.
    FaceMap face_cache{};
    ReplacementMap replacement_cache{};

    auto face = [this, &editor, &face_cache](const std::string_view name) -> std::optional<Face> {
        if (const auto it = face_cache.find(name); it != face_cache.end()) { return it->second; }

        auto ret = editor.resolve_face(name, *this);
        if (ret) { face_cache.emplace(name, *ret); }
        return ret;
    };

    auto replacement = [this, &editor, &replacement_cache](const std::string_view name)-> std::optional<Replacement> {
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

    // Draw main content.
    for (std::size_t y = 0; y < height; y += 1) {
        const auto doc_y = this->scroll_.row_ + y;

        // Draw gutter.
        if (this->gutter_) {
            if (doc_y < this->doc_->line_count()) {
                // 32 characters for the line number should be plenty.
                char line_num[32]{};
                auto [ptr, ec] = std::to_chars(line_num, line_num + sizeof(line_num), doc_y + 1);
                std::size_t len = ptr - line_num;
                auto padding = gutter_width - 1 - len;

                Cell c(' ', *gutter_face);

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
                Cell c(' ', *gutter_face);
                for (std::size_t x = 0; x < std::min(gutter_width, this->width_); x += 1) {
                    display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
                }
            }
        }

        // Draw document content.
        if (doc_y < this->doc_->line_count()) {
            auto line = this->doc_->line(doc_y);
            const auto syntax_overlay = this->generated_syntax_overlay(editor, line);

            std::size_t x = 0;
            std::size_t idx = 0;
            while (idx < line.size()) {
                const auto len = util::utf8::len(line[idx]);
                // Draw the replacement character on malformed input.
                const auto ch = idx + len <= line.size() ? line.substr(idx, len) : "�";

                Cell cell("", *default_face);

                // Layer 1: Syntax highlighting.
                if (syntax_overlay[idx]) { if (const auto f = face(*syntax_overlay[idx]); f) { cell.set_face(*f); } }

                // Layer 2: Character replacement.
                if (const auto r = replacement(ch); r) {
                    cell.set_utf8(r->txt);
                    if (const auto f = face(r->face); f) { cell.set_face(*f); }
                } else { cell.set_utf8(ch); }

                const auto width = util::char_width(ch, x, this->doc_->tab_width_);
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
            Cell c(' ', *default_face);
            for (; x < this->scroll_.col_ + content_width; x += 1) {
                if (x >= this->scroll_.col_) {
                    const std::size_t screen_x = x - this->scroll_.col_ + gutter_width;
                    display.update(this->offset_.col_ + screen_x, this->offset_.row_ + y, c);
                }
            }
        } else { // Fill the empty line.
            Cell c(' ', *default_face);
            for (std::size_t x = gutter_width; x < this->width_; x += 1) {
                display.update(this->offset_.col_ + x, this->offset_.row_ + y, c);
            }
        }
    }

    if (this->mode_line_) { this->render_mode_line(display, editor); }
}

void Viewport::render_cursor(Display& display) const {
    // Don't draw cursors outside the viewport.
    auto height = this->height_;
    if (this->mode_line_ && this->mode_line_renderer_.valid()) {
        height = util::math::sub_sat(this->height_, static_cast<std::size_t>(1));
    }
    if (this->cur_.pos_.row_ < this->scroll_.row_ || this->cur_.pos_.row_ >= this->scroll_.row_ + height) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    const auto y = this->cur_.pos_.row_ - this->scroll_.row_;
    const auto line = this->doc_->line(this->cur_.pos_.row_);
    std::size_t x = 0;
    std::size_t idx = 0;
    while (idx < line.size() && idx < this->cur_.pos_.col_) {
        const auto len = util::utf8::len(line[idx]);
        const auto width = util::char_width(line.substr(idx, len), x, this->doc_->tab_width_);

        x += width;
        idx += len;
    }

    // Don't draw cursors outside the viewport.
    if (const auto content_width = util::math::sub_sat(this->width_, gutter);
        x < this->scroll_.col_ || x >= this->scroll_.col_ + content_width) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }
    x = x - this->scroll_.col_;

    display.cursor(this->offset_.row_ + y, this->offset_.col_ + gutter + x);
}

void Viewport::adjust_viewport() {
    if (this->doc_->line_count() == 0) return;

    // 1. Vertical scrolling.
    auto height = this->height_;
    if (this->mode_line_ && this->mode_line_renderer_.valid()) {
        height = util::math::sub_sat(this->height_, static_cast<std::size_t>(1));
    }
    if (this->cur_.pos_.row_ < this->scroll_.row_) { // Above.
        this->scroll_.row_ = this->cur_.pos_.row_;
    } else if (this->cur_.pos_.row_ >= this->scroll_.row_ + height) { // Bellow.
        this->scroll_.row_ = this->cur_.pos_.row_ - height + 1;
    }

    if (this->cur_.pos_.row_ >= this->doc_->line_count()) return;

    // 2. Horizontal scrolling.
    const auto line = this->doc_->line(this->cur_.pos_.row_);
    std::size_t x = 0;
    std::size_t idx = 0;
    while (idx < line.size()) {
        const auto len = util::utf8::len(line[idx]);
        const auto width = util::char_width(line.substr(idx, len), x, this->doc_->tab_width_);

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
        this->scroll_.col_ = x - util::math::sub_sat(this->width_, gutter) + 1;
    }
}

void Viewport::render_mode_line(Display& display, const Editor& editor) const {
    const auto res = this->mode_line_renderer_(*this);
    // TODO: log errors.
    if (!res.valid() || res.get_type() != sol::type::table) { return; }

    sol::table segments = res;
    const std::size_t row = this->offset_.row_ + this->height_ - 1;

    auto text_width = [*this](const std::string_view text, const std::size_t offset) {
        std::size_t width = 0;
        std::size_t idx = 0;

        while (idx < text.size()) {
            const auto len = util::utf8::len(text[idx]);
            const auto ch = text.substr(idx, len);
            width += util::char_width(ch, offset + width, this->doc_->tab_width_);
            idx += len;
        }

        return width;
    };

    std::size_t total_width = 0;
    std::size_t num_spacers = 0;
    std::size_t last_spacer_idx = 0;
    for (std::size_t idx = 1; idx <= segments.size(); idx += 1) {
        if (sol::table segment = segments[idx]; segment["spacer"].get_or(false)) { // Count spacers.
            num_spacers += 1;
            last_spacer_idx = idx;
        } else { // Calculate text width.
            const std::string text = segment["text"].get_or(std::string{});
            total_width += text_width(text, total_width);
        }
    }

    std::size_t spacer_width = 0;
    std::size_t spacer_remainder = 0;
    if (num_spacers > 0 && total_width < this->width_) {
        spacer_width = (this->width_ - total_width) / num_spacers;
        spacer_remainder = (this->width_ - total_width) % num_spacers;
    }

    // Draw segments.
    std::size_t curr = 0;
    for (std::size_t idx = 1; idx <= segments.size(); idx += 1) {
        sol::table seg = segments[idx];
        const auto face = editor.resolve_face(seg["face"].get_or(std::string("default")), *this);

        std::string text;
        if (seg["spacer"].get_or(false)) {
            auto width = spacer_width;

            if (spacer_remainder > 0) {
                width += 1;
                spacer_remainder -= 1;
            }

            text.assign(width, ' ');
        } else { text = seg["text"].get_or(std::string{}); }

        // Draw text.
        std::size_t jdx = 0;
        while (jdx < text.size()) {
            const auto len = util::utf8::len(text[jdx]);
            const auto ch = text.substr(jdx, len);
            const auto width = util::char_width(ch, curr, this->doc_->tab_width_);

            if (curr + width > this->width_) { break; }

            Cell c(ch, *face);
            display.update(this->offset_.col_ + curr, row, c);

            if (width > 1) {
                Cell filler("", *face);
                for (std::size_t n = 1; n < width; n += 1) {
                    display.update(this->offset_.col_ + curr + n, row, filler);
                }
            }

            curr += width;
            jdx += len;
        }
    }

    // Fill remainder of line.
    if (curr < this->width_) {
        Cell c(' ', *editor.resolve_face("default", *this));
        for (; curr < this->width_; curr += 1) { display.update(this->offset_.col_ + curr, row, c); }
    }

    // On overflow, redraw the last segments as it usually contains crucial information.
    if (total_width >= this->width_ && last_spacer_idx > 0 && last_spacer_idx < segments.size()) {
        std::size_t dock_width = 0;
        for (std::size_t idx = last_spacer_idx + 1; idx <= segments.size(); idx += 1) {
            sol::table seg = segments[idx];
            dock_width += text_width(seg["text"].get_or(std::string{}), dock_width);
        }

        curr = util::math::sub_sat(this->width_, dock_width);
        for (std::size_t idx = last_spacer_idx + 1; idx <= segments.size(); idx += 1) {
            sol::table seg = segments[idx];
            const auto face = editor.resolve_face(seg["face"].get_or(std::string("default")), *this);

            const std::string text = seg["text"].get_or(std::string{});

            std::size_t jdx = 0;
            while (jdx < text.size()) {
                const auto len = util::utf8::len(text[jdx]);
                const auto ch = text.substr(jdx, len);
                const auto width = util::char_width(ch, curr, this->doc_->tab_width_);

                if (curr + width > this->width_) { break; }

                Cell c(ch, *face);
                display.update(this->offset_.col_ + curr, row, c);

                if (width > 1) {
                    Cell filler("", *face);
                    for (std::size_t n = 1; n < width; n += 1) {
                        display.update(this->offset_.col_ + curr + n, row, filler);
                    }
                }

                curr += width;
                jdx += len;
            }
        }
    }
}

std::vector<const std::string*> Viewport::generated_syntax_overlay(const Editor& editor,
                                                                   const std::string_view line) const {
    std::vector<const std::string*> overlay(line.size(), nullptr);

    auto apply_mode = [line, &overlay](const std::shared_ptr<Mode>& mode) {
        for (const auto& [pattern, face]: mode->syntax_rules_) {
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
    if (this->doc_->major_mode_) {
        apply_mode(this->doc_->major_mode_);
    }
    // 2. Global Minor Modes (evaluated front to back for precedence of later Minor Modes on the stack).
    for (const auto& mode: editor.get_global_minor_modes()) { apply_mode(mode); }
    // 3. Document Minor Modes (evaluated front to back for precedence of later Minor Modes on the stack).
    for (const auto& mode: this->doc_->minor_modes_) { apply_mode(mode); }

    return overlay;
}

