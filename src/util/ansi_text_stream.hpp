#ifndef ANSI_TEXT_STREAM_HPP_
#define ANSI_TEXT_STREAM_HPP_

#include <memory>
#include <string>

#include <sol/object.hpp>

struct Document;

/// A stateful ANSI text parser that sets a Documents text properties.
struct AnsiTextStream {
private:
    enum struct StyleMask : std::uint8_t {
        NONE = 0,
        BOLD = 1,
        ITALIC = 2,
        UNDERLINE = 4,
        STRIKETHROUGH = 8,
    };

    // The Document in which to input the stylized text.
    std::shared_ptr<Document> doc_;
    // Remaining previous chunk that couldnt be parsed fully.
    std::string buffer_{};

    sol::object fg_{sol::lua_nil};
    sol::object bg_{sol::lua_nil};
    sol::object style_{sol::lua_nil};
    uint8_t style_mask_{0};

    std::unordered_map<std::size_t, sol::object> fg_cache_{};
    std::unordered_map<std::size_t, sol::object> bg_cache_{};
    std::array<sol::object, 16> style_cache_{};

    std::unordered_map<uint32_t, sol::object> rgb_fg_cache_{};
    std::unordered_map<uint32_t, sol::object> rgb_bg_cache_{};

public:
    explicit AnsiTextStream(std::shared_ptr<Document> doc);

    /// Statefully parses an ANSI text stream into a Document at a specified position.
    auto parse(std::string_view text, std::size_t pos) -> std::size_t;

private:
    void process_sgr(std::string_view sgr_codes);

    auto get_fg(std::size_t code) -> sol::object;
    auto get_bg(std::size_t code) -> sol::object;
    auto get_style() -> sol::object;

    auto get_rgb_fg(uint8_t r, uint8_t g, uint8_t b) -> sol::object;
    auto get_rgb_bg(uint8_t r, uint8_t g, uint8_t b) -> sol::object;
};

#endif
