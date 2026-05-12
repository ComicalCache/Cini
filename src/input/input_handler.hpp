#ifndef INPUT_HANDLER_HPP_
#define INPUT_HANDLER_HPP_

#include <string_view>
#include <variant>
#include <vector>

#include "ansi_parser.hpp"
#include "key_event.hpp"
#include "mouse_event.hpp"

struct InputHandler {
public:
    using Event = std::variant<KeyEvent, MouseEvent>;

private:
    AnsiParser parser_{};
    std::vector<InputHandler::Event> events_{};

    std::string utf8_ch_{};

public:
    InputHandler();

    [[nodiscard]]
    auto parse(std::string_view data) -> std::vector<InputHandler::Event>;

private:
    void setup_callbacks();
};

#endif
