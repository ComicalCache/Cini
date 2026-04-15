#include "bindings.hpp"

#include <sol/table.hpp>

#include "../types/face.hpp"

void FaceBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Face>("Face",
        /* Properties. */
        "fg", sol::property(
            [](const Face& face) -> std::optional<Rgb> { return face.fg_; },
            [](Face& face, const std::optional<Rgb> fg) -> void { face.fg_ = fg; }
        ),
        "bg", sol::property(
            [](const Face& face) -> std::optional<Rgb> { return face.bg_; },
            [](Face& face, const std::optional<Rgb> bg) -> void { face.bg_ = bg; }
        ),
        "bold", sol::property(
            [](const Face& face) -> std::optional<bool> { return face.bold_; },
            [](Face& face, const std::optional<bool> bold) -> void { face.bold_ = bold; }
        ),
        "italic", sol::property(
            [](const Face& face) -> std::optional<bool> { return face.italic_; },
            [](Face& face, const std::optional<bool> italic) -> void { face.italic_ = italic; }
        ),
        "underline", sol::property(
            [](const Face& face) -> std::optional<bool> { return face.underline_; },
            [](Face& face, const std::optional<bool> underline) -> void { face.underline_ = underline; }
        ),
        "strikethrough", sol::property(
            [](const Face& face) -> std::optional<bool> { return face.strikethrough_; },
            [](Face& face, const std::optional<bool> strikethrough) -> void { face.strikethrough_ = strikethrough; }
        ),

        /* Functions. */
        sol::call_constructor, sol::factories(
            [] -> Face { return Face{}; },
            [](const sol::table& table) -> Face {
                Face f{};
                if (const auto fg = table["fg"]; fg.valid() && fg.is<Rgb>()) { f.fg_ = fg.get<Rgb>(); }
                if (const auto bg = table["bg"]; bg.valid() && bg.is<Rgb>()) { f.bg_ = bg.get<Rgb>(); }
                if (const auto b = table["bold"]; b.valid() && b.is<bool>()) { f.bold_ = b.get<bool>(); }
                if (const auto i = table["italic"]; i.valid() && i.is<bool>()) { f.italic_ = i.get<bool>(); }
                if (const auto u = table["underline"]; u.valid() && u.is<bool>()) { f.underline_ = u.get<bool>(); }
                if (const auto s = table["strikethrough"]; s.valid() && s.is<bool>()) {
                    f.strikethrough_ = s.get<bool>();
                }
                return f;
            }
        ),

        "clone", [](const Face& self) -> Face { return {self}; });
    // clang-format on
}
