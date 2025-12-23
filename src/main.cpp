#include <print>

#include "ansi.hpp"
#include "editor.hpp"

int main() {
    std::setlocale(LC_ALL, "");

    std::string s{};
    ansi::alt_screen(s);
    std::println("{}", s);

    Editor{}.init_uv().init_lua().init_bridge().init_state().run();

    s.clear();
    ansi::main_screen(s);
    std::println("{}", s);

    return 0;
}
