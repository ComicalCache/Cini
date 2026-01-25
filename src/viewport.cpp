#include "viewport.hpp"

#include "document.hpp"
#include "editor.hpp"
#include "face_cache.hpp"
#include "rendering/window.hpp"
#include "types/face.hpp"
#include "util/assert.hpp"
#include "util/math.hpp"
#include "util/utf8.hpp"

auto Viewport::find_viewport(const std::shared_ptr<Window>& node) -> std::shared_ptr<Viewport> {
    if (node->viewport_) { return node->viewport_; }

    // Always prefer the first child if not leaf.
    return find_viewport(node->child_1_);
}

Viewport::Viewport(const std::size_t width, const std::size_t height, std::shared_ptr<Document> doc)
    : doc_{std::move(doc)}, width_{width}, height_{height} {
    ASSERT(this->doc_ != nullptr, "viewport must hold a doc");
}

void Viewport::move_cursor(const cursor::move_fn& move_fn, const std::size_t n) {
    move_fn(this->cur_, *this->doc_, n);
    this->doc_->point_ = this->cur_.point(*this->doc_);
    this->adjust_viewport();
}

void Viewport::scroll_up(const std::size_t n) { this->scroll_.row_ = math::sub_sat(this->scroll_.row_, n); }

void Viewport::scroll_down(const std::size_t n) {
    if (const auto max_scroll = this->doc_->line_count(); this->scroll_.row_ + n < max_scroll) {
        this->scroll_.row_ += n;
    } else {
        this->scroll_.row_ = max_scroll;
    }
}

void Viewport::scroll_left(const std::size_t n) { this->scroll_.col_ = math::sub_sat(this->scroll_.col_, n); }

void Viewport::scroll_right(const std::size_t n) { this->scroll_.col_ += n; }

void Viewport::resize(const std::size_t width, const std::size_t height, const Position offset) {
    if (this->width_ == width && this->height_ == height &&                     // Dimensions.
        this->offset_.row_ == offset.row_ && this->offset_.col_ == offset.col_) // Offset.
    {
        return;
    }

    this->width_ = width;
    this->height_ = height;
    this->offset_ = offset;

    this->adjust_viewport();
}

auto Viewport::render(Display& display, const sol::protected_function& resolve_face) -> bool {
    if (this->mode_line_ && !this->mode_line_callback_.valid()) {
        // Triggers a rerender.
        this->mode_line_ = false;
        // TODO: log error.
        return false;
    }

    auto height = this->mode_line_ ? math::sub_sat(this->height_, static_cast<std::size_t>(1)) : this->height_;
    if (height == 0) { return false; }

    Face default_face{};
    if (const auto f = resolve_face(this->doc_, "default"); f.valid()) {
        default_face = f.get<Face>();
    } else {
        ASSERT(false, "default face must be defined");
    }
    Face gutter_face{};
    if (const auto f = resolve_face(this->doc_, "gutter"); f.valid()) {
        gutter_face = f.get<Face>();
    } else {
        ASSERT(false, "gutter face must be defined");
    }
    Face replacement_face{};
    if (const auto f = resolve_face(this->doc_, "replacement"); f.valid()) {
        replacement_face = f.get<Face>();
    } else {
        ASSERT(false, "replacement face must be defined");
    }

    std::size_t gutter_width = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter_width = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }
    if (this->width_ <= gutter_width) { return false; }

    const auto content_width = math::sub_sat(this->width_, gutter_width);
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = this->doc_->properties_["tab_width"]; t) { tab_width = *t; }

    const auto ws = static_cast<sol::optional<std::string_view>>(this->doc_->properties_["ws"]).value_or(" ");
    const auto nl = static_cast<sol::optional<std::string_view>>(this->doc_->properties_["nl"]).value_or(" ");
    const auto tab = static_cast<sol::optional<std::string_view>>(this->doc_->properties_["tab"]).value_or(" ");

    Face ws_face{};
    if (const auto f = resolve_face(this->doc_, "ws"); f.valid()) {
        ws_face = f.get<Face>();
    } else {
        ws_face = default_face;
    }
    Face nl_face{};
    if (const auto f = resolve_face(this->doc_, "nl"); f.valid()) {
        nl_face = f.get<Face>();
    } else {
        nl_face = default_face;
    }
    Face tab_face{};
    if (const auto f = resolve_face(this->doc_, "tab"); f.valid()) {
        tab_face = f.get<Face>();
    } else {
        tab_face = default_face;
    }

    std::optional<FaceCache> face_cache{std::nullopt};

    this->visual_cur_ = std::nullopt;
    const auto cur_byte = this->cur_.point(*this->doc_);

    std::size_t logical_y{1};
    std::size_t last_rendered_gutter_y{0};
    std::size_t x{0};
    std::size_t y{0};
    std::size_t idx{0};

    auto fill_line = [&] -> void {
        if (y >= this->scroll_.row_) {
            for (std::size_t n = std::max(x, this->scroll_.col_); n < this->scroll_.col_ + content_width; n += 1) {
                this->_draw_char(display, default_face, gutter_width, content_width, " ", 1, false, n, y);
            }
        }
    };

    auto draw = [&](const std::string_view ch, const bool replacement) -> void {
        auto draw_ch = ch;
        auto term_width = utf8::char_width(ch, x, tab_width);

        if (y >= this->scroll_.row_ && x + term_width >= this->scroll_.col_ && x < this->scroll_.col_ + content_width) {
            if (!face_cache) { face_cache.emplace(idx, *this->doc_); }

            face_cache->update(idx, [&](const std::string_view name) -> sol::optional<Face> {
                return resolve_face(this->doc_, name);
            });

            auto face = default_face;
            if (face_cache->face_) { face.merge(*face_cache->face_); }
            if (replacement && cur_byte == idx) { face.merge(replacement_face); }

            if (ch == " ") {
                draw_ch = ws;
                face.merge(ws_face);
                term_width = utf8::char_width(draw_ch, x, tab_width);
            } else if (ch == "\n") {
                draw_ch = nl;
                face.merge(nl_face);
                term_width = utf8::char_width(draw_ch, x, tab_width);
            } else if (ch == "\t") {
                draw_ch = tab;
                face.merge(tab_face);
                // Subtracting the character width, adding one for the width of the previous tab replacement.
                term_width -= utf8::char_width(draw_ch, x, tab_width) - 1;
            }

            this->_draw_char(display, face, gutter_width, content_width, draw_ch, term_width, ch == "\t", x, y);
        }

        x += term_width;

        if (ch == "\n") {
            fill_line();
            x = 0;
            y += 1;

            if (replacement && this->gutter_ && y >= this->scroll_.row_ && y < this->scroll_.row_ + height) {
                this->_draw_gutter(display, gutter_face, gutter_width, std::nullopt, y);
            }
        }
    };

    while (y < this->scroll_.row_ + height && idx < this->doc_->size()) {
        if (cur_byte == idx) { this->visual_cur_ = {.row_ = y, .col_ = x}; }

        // Only draw the gutter if it hasn't been drawn yet for this line.
        if (this->gutter_ && y >= this->scroll_.row_ && last_rendered_gutter_y != logical_y) {
            this->_draw_gutter(display, gutter_face, gutter_width, logical_y, y);
            last_rendered_gutter_y = logical_y;
        }

        if (const auto* const replacement = this->doc_->get_raw_text_property(idx, "replacement"); replacement) {
            const auto contents = replacement->value_.as<std::string_view>();

            std::size_t jdx = 0;
            while (y < this->scroll_.row_ + height && jdx < contents.size()) {
                const auto ch_len = utf8::len(contents[jdx]);
                const auto ch = jdx + ch_len <= contents.size() ? contents.substr(jdx, ch_len) : "";

                draw(ch, true);

                jdx += ch_len;
            }

            idx += replacement->end_ - replacement->start_;
            logical_y += std::ranges::count(this->doc_->slice(replacement->start_, replacement->end_), '\n');
        } else {
            const auto ch_len = utf8::len(this->doc_->slice(idx, idx + 1)[0]);
            const auto ch = idx + ch_len <= this->doc_->size() ? this->doc_->slice(idx, idx + ch_len) : "";

            draw(ch, false);

            if (ch == "\n") { logical_y += 1; }
            idx += ch_len;
        }
    }

    if (cur_byte == idx) { this->visual_cur_ = {.row_ = y, .col_ = x}; }

    // Fill last drawn remaining line.
    if (y < this->scroll_.row_ + height && y >= this->scroll_.row_ && x != 0) {
        fill_line();
        y += 1;
    }

    // Fill the trailing lines.
    while (y < this->scroll_.row_ + height) {
        if (y >= this->scroll_.row_) {
            // Draw empty gutter.
            if (this->gutter_) { this->_draw_gutter(display, gutter_face, gutter_width, std::nullopt, y); }

            x = 0;
            fill_line();
        }

        y += 1;
    }

    if (this->mode_line_) { return this->render_mode_line(display, resolve_face); }
    return true;
}

auto Viewport::render_mode_line(Display& display, const sol::protected_function& resolve_face) -> bool {
    const auto res = this->mode_line_callback_(*this);
    if (!res.valid() || res.get_type() != sol::type::table) {
        sol::error err{"Expected a Mode Line table."};
        if (!res.valid()) { err = res; }

        // Triggers a rerender.
        this->mode_line_ = false;
        // TODO: log error.

        return false;
    }

    Face mode_line_face{};
    if (const auto f = resolve_face(this->doc_, "mode_line"); f.valid()) {
        mode_line_face = f.get<Face>();
    } else {
        ASSERT(false, "mode_line face must be defined");
    }
    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = this->doc_->properties_["tab_width"]; t) { tab_width = *t; }

    sol::table segments = res;
    const std::size_t y = this->height_ + this->scroll_.row_ - 1;

    std::size_t total_width = 0;
    std::size_t num_spacers = 0;
    std::size_t last_spacer_idx = 0;
    for (std::size_t idx = 1; idx <= segments.size(); idx += 1) {
        if (sol::table segment = segments[idx]; segment["spacer"].get_or(false)) { // Count spacers.
            num_spacers += 1;
            last_spacer_idx = idx;
        } else { // Calculate text width.
            const std::string text = segment["text"].get_or(std::string{});
            total_width += utf8::str_width(text, total_width, tab_width);
        }
    }

    std::size_t spacer_width = 0;
    std::size_t spacer_remainder = 0;
    if (num_spacers > 0 && total_width < this->width_) {
        spacer_width = (this->width_ - total_width) / num_spacers;
        spacer_remainder = (this->width_ - total_width) % num_spacers;
    }

    auto get_face = [&](const sol::table& segment) -> Face {
        auto face = mode_line_face;
        if (segment["face"].is<Face>()) {
            face.merge(segment["face"].get<Face>());
        } else if (segment["face"].is<std::string_view>()) {
            if (const auto f = resolve_face(this->doc_, segment["face"].get<std::string_view>()); f.valid()) {
                face.merge(f.get<Face>());
            }
        }

        return face;
    };

    auto draw_text = [&](std::size_t& curr, const std::string_view& text, const Face& face) -> void {
        std::size_t jdx = 0;
        while (jdx < text.size()) {
            const auto len = utf8::len(text[jdx]);
            const auto ch = text.substr(jdx, len);
            const auto width = utf8::char_width(ch, curr, tab_width);

            this->_draw_char(display, face, 0, this->width_, ch, width, ch == "\t", curr + this->scroll_.col_, y);

            curr += width;
            jdx += len;

            if (curr >= this->width_) { break; }
        }
    };

    // Draw segments.
    std::size_t curr = 0;
    for (std::size_t idx = 1; idx <= segments.size(); idx += 1) {
        sol::table segment = segments[idx];
        auto face = get_face(segment);

        if (segment["spacer"].get_or(false)) {
            auto width = spacer_width;
            if (spacer_remainder > 0) {
                width += 1;
                spacer_remainder -= 1;
            }

            for (std::size_t jdx = 0; jdx < width; jdx += 1) { draw_text(curr, " ", face); }
        } else {
            draw_text(curr, segment["text"].get_or(std::string_view{}), face);
        }
    }

    // Fill remainder of line.
    if (curr < this->width_) {
        while (curr < this->width_) {
            this->_draw_char(display, mode_line_face, 0, this->width_, " ", 1, false, curr + this->scroll_.col_, y);
            curr += 1;
        }
    }

    // On overflow, redraw the last segments as it usually contains crucial information.
    if (total_width >= this->width_ && last_spacer_idx > 0 && last_spacer_idx < segments.size()) {
        std::size_t dock_width = 0;
        for (std::size_t idx = last_spacer_idx + 1; idx <= segments.size(); idx += 1) {
            sol::table seg = segments[idx];
            dock_width += utf8::str_width(seg["text"].get_or(std::string_view{}), dock_width, tab_width);
        }

        curr = math::sub_sat(this->width_, dock_width);
        for (std::size_t idx = last_spacer_idx + 1; idx <= segments.size(); idx += 1) {
            sol::table segment = segments[idx];
            auto face = get_face(segment);

            draw_text(curr, segment["text"].get_or(std::string_view{}), face);
        }
    }

    return true;
}

void Viewport::render_cursor(Display& display) const {
    if (!this->visual_cur_) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }

    // Since the visual cursor uses the logical y it could be scrolled off.
    if (this->visual_cur_->row_ < this->scroll_.row_) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }

    const auto x = this->visual_cur_->col_;
    const auto y = this->visual_cur_->row_ - this->scroll_.row_;

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    // Horizontal check.
    if (const auto content_width = math::sub_sat(this->width_, gutter);
        x < this->scroll_.col_ || x >= this->scroll_.col_ + content_width) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }

    // Vertical check.
    auto height = this->height_;
    if (this->mode_line_ && this->mode_line_callback_.valid()) {
        height = math::sub_sat(this->height_, static_cast<std::size_t>(1));
    }
    if (y >= height) {
        display.cursor(0, 0, ansi::CursorStyle::HIDDEN);
        return;
    }

    display.cursor(this->offset_.row_ + y, this->offset_.col_ + gutter + (x - this->scroll_.col_));
}

void Viewport::adjust_viewport() {
    if (this->doc_->line_count() == 0) { return; }

    auto tab_width = static_cast<std::size_t>(4);
    if (const sol::optional<std::size_t> t = this->doc_->properties_["tab_width"]; t) { tab_width = *t; }

    // 1. Vertical scrolling.
    auto height = this->height_;
    if (this->mode_line_ && this->mode_line_callback_.valid()) {
        height = math::sub_sat(this->height_, static_cast<std::size_t>(1));
    }
    if (this->cur_.pos_.row_ < this->scroll_.row_) { // Above.
        this->scroll_.row_ = this->cur_.pos_.row_;
    } else if (this->cur_.pos_.row_ >= this->scroll_.row_ + height) { // Bellow.
        this->scroll_.row_ = this->cur_.pos_.row_ - height + 1;
    }

    if (this->cur_.pos_.row_ >= this->doc_->line_count()) { return; }

    // 2. Horizontal scrolling.
    const auto line = this->doc_->line(this->cur_.pos_.row_);
    const auto x = utf8::str_width(line.substr(0, std::min(this->cur_.pos_.col_, line.size())), 0, tab_width);

    std::size_t gutter = 0;
    if (this->gutter_) {
        const auto total_lines = this->doc_->line_count();
        gutter = (total_lines > 0 ? static_cast<size_t>(std::log10(total_lines)) + 1 : 1) + 2;
    }

    if (x < this->scroll_.col_) { // Left.
        this->scroll_.col_ = x;
    } else if (x >= this->scroll_.col_ + this->width_ - gutter) { // Right.
        this->scroll_.col_ = x - math::sub_sat(this->width_, gutter) + 1;
    }
}

void Viewport::_draw_gutter(
    Display& display, const Face face, const std::size_t gutter_width, const std::optional<std::size_t> line,
    const std::size_t y) const {
    const auto vy = y - this->scroll_.row_;

    if (line) {
        // 32 characters for the line number should be plenty.
        std::array<char, 32> line_num{};
        auto [ptr, ec] = std::to_chars(line_num.data(), line_num.data() + line_num.size(), *line);
        const std::size_t len = ptr - line_num.data();
        const auto padding = gutter_width - 1 - len;

        Cell c(' ', face);

        // Draw padding.
        for (std::size_t x = 0; x < std::min(padding, this->width_); x += 1) {
            display.update(this->offset_.col_ + x, this->offset_.row_ + vy, c);
        }
        // Draw number.
        for (std::size_t x = padding; x < std::min(padding + len, this->width_); x += 1) {
            c.set_char(line_num[x - padding]);
            display.update(this->offset_.col_ + x, this->offset_.row_ + vy, c);
        }
        // Draw trailing space.
        if (padding + len < this->width_) {
            c.set_char(' ');
            display.update(this->offset_.col_ + padding + len, this->offset_.row_ + vy, c);
        }
    } else { // Draw empty gutter.
        const Cell c(' ', face);
        for (std::size_t x = 0; x < std::min(gutter_width, this->width_); x += 1) {
            display.update(this->offset_.col_ + x, this->offset_.row_ + vy, c);
        }
    }
}

void Viewport::_draw_char(
    Display& display, const Face face, const std::size_t gutter_width, const std::size_t content_width,
    const std::string_view ch, const std::size_t width, const bool tab, const std::size_t x,
    const std::size_t y) const {
    const Cell cell(ch, face);

    const auto vy = y - this->scroll_.row_;
    for (std::size_t n = 0; n < width; n += 1) {
        // Visual x position.
        auto vx = x + n;

        // Visual x position before visible section.
        if (vx < this->scroll_.col_) { continue; }
        // Visual x position after visible section.
        if (vx >= this->scroll_.col_ + content_width) { break; }

        // Visible section x position.
        vx = vx - this->scroll_.col_;
        if (n == 0) {
            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + vy, cell);
        } else { // Expand tab or wide characters.
            Cell filler("", cell.fg_, cell.bg_);
            if (tab) {
                filler.set_char(' ');
            } else if (x < this->scroll_.col_) { // Half-cutoff wide character.
                filler.set_utf8("▯");
            }

            display.update(this->offset_.col_ + gutter_width + vx, this->offset_.row_ + vy, filler);
        }
    }
}
