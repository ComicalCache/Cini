#include <csignal>
#include <print>

#include "ansi.hpp"
#include "editor.hpp"

void signal_handler(const int signum) {
    uv_tty_reset_mode();

    // Reraise to generate dump.
    std::signal(signum, SIG_DFL);
    std::raise(signum);
}

int main() {
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGILL, signal_handler);
    std::signal(SIGFPE, signal_handler);

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
