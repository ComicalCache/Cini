#include "face.hpp"

void Face::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Face>("Face",
        "fg", sol::property(
            [](const Face& face) { return face.fg_; },
            [](Face& face, const std::optional<Rgb> fg) { face.fg_ = fg; }
        ),
        "bg", sol::property(
            [](const Face& face) { return face.bg_; },
            [](Face& face, const std::optional<Rgb> bg) { face.bg_ = bg; }
        ),
        sol::call_constructor, sol::factories(
            [] { return Face{}; },
            [](const sol::table& table) {
                Face f;
                if (const auto fg = table["fg"]; fg.valid() && fg.is<Rgb>()) { f.fg_ = fg.get<Rgb>(); }
                if (const auto bg = table["bg"]; bg.valid() && bg.is<Rgb>()) { f.bg_ = bg.get<Rgb>(); }
                return f;
            }
        ));
    // clang-format on
}

void Face::merge(const Face& other) {
    if (other.fg_) { this->fg_ = other.fg_; }
    if (other.bg_) { this->bg_ = other.bg_; }
}
