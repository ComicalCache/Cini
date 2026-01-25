#include "../../types/face.hpp"

#include <sol/table.hpp>

void Face::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Face>("Face",
        /* Properties. */
        /// Foreground color.
        "fg", sol::property(
            [](const Face& face) -> std::optional<Rgb> { return face.fg_; },
            [](Face& face, const std::optional<Rgb> fg) -> void { face.fg_ = fg; }
        ),
        /// Background color.
        "bg", sol::property(
            [](const Face& face) -> std::optional<Rgb> { return face.bg_; },
            [](Face& face, const std::optional<Rgb> bg) -> void { face.bg_ = bg; }
        ),

        /* Functions. */
        sol::call_constructor, sol::factories(
            [] -> Face { return Face{}; },
            [](const sol::table& table) -> Face {
                Face f{};
                if (const auto fg = table["fg"]; fg.valid() && fg.is<Rgb>()) { f.fg_ = fg.get<Rgb>(); }
                if (const auto bg = table["bg"]; bg.valid() && bg.is<Rgb>()) { f.bg_ = bg.get<Rgb>(); }
                return f;
            }
        ));
    // clang-format on
}
