#include <clocale>
#include <csignal>
#include <iostream>
#include <print>
#include <sstream>
#include <sys/fcntl.h>
#include <unistd.h>

#include "cli_parser.hpp"
#include "editor.hpp"
#include "gen/lua_defaults.hpp"
#include "gen/version.hpp"
#include "util/ansi.hpp"
#include "util/fs.hpp"

constexpr auto HELP_MSG = "Usage: ./{} [ARGS] [PATH]\n"
                          "\n"
                          "ARGS:\n"
                          "    --help\n"
                          "        this message\n"
                          "    --version\n"
                          "        prints version information\n"
                          "    --defaults\n"
                          "        dumps the default configuration in a folder `defaults` in the current directory\n"
                          "    --mode=MODE_NAME\n"
                          "        opens the first document with mode MODE_NAME\n"
                          "\n"
                          "PATH:\n"
                          "    Path to the file to open or nothing to create a new scratchpad\n";

void signal_handler(const int signum) {
    uv_tty_reset_mode();

    std::string s{};
    ansi::main_screen(s);
    ansi::disable_kitty_protocol(s);
    std::print("{}", s);

    // Reraise to generate dump.
    std::signal(signum, SIG_DFL);
    std::raise(signum);
}

auto main(const int argc, char* argv[]) -> int {
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGILL, signal_handler);
    std::signal(SIGFPE, signal_handler);

    std::setlocale(LC_ALL, "");

    std::string piped{};
    if (isatty(STDIN_FILENO) == 0) {
        std::ostringstream oss{};
        oss << std::cin.rdbuf();
        piped = oss.str();

        auto tty_fd = open("/dev/tty", O_RDONLY);
        if (tty_fd != -1) {
            dup2(tty_fd, STDIN_FILENO);
            close(tty_fd);
        } else {
            std::println("Failed to reopen /dev/tty");
            return 1;
        }

        std::cin.clear();
    }

    Editor::bootstrap();
    CliParser cli(argc, argv, Editor::instance()->lua_.create_table());

    if (!piped.empty()) { cli.options_["piped"] = std::move(piped); }

    if (cli.options_["help"].get_or(false)) {
        std::print(HELP_MSG, std::filesystem::path{argv[0]}.filename().c_str());
        return 0;
    }
    if (cli.options_["version"].get_or(false)) {
        std::println(
            "{} v{}, built on {} [{}]", version::NAME, version::VERSION, version::BUILD_DATE, version::BUILD_TYPE);
        return 0;
    }
    if (cli.options_["defaults"].get_or(false)) {
        const auto base = std::filesystem::current_path() / "defaults";
        for (const auto& [module_name, content]: lua_modules::files) {
            std::string path_str = std::string{module_name};
            std::ranges::replace(path_str, '.', std::filesystem::path::preferred_separator);
            path_str += ".lua";

            const auto full_path = base / path_str;
            if (const auto parent = full_path.parent_path(); !parent.empty()) {
                std::error_code ec{};
                std::filesystem::create_directories(parent, ec);
                if (ec) {
                    std::println("{}", ec.message());
                    return 1;
                }
            }

            if (!fs::write_file(full_path, content, std::ios::out | std::ios::trunc)) {
                std::println("Failed to write to file '{}'", full_path.string());
                return 1;
            }
        }

        std::println("Default config has been written to '{}'", base.string());
        return 0;
    }

    std::string s{};
    ansi::alt_screen(s);
    ansi::enable_kitty_protocol(s);
    std::print("{}", s);
    std::fflush(stdout);

    Editor::setup(std::move(cli));
    Editor::run();
    Editor::destroy();

    s.clear();
    ansi::main_screen(s);
    ansi::disable_kitty_protocol(s);
    std::print("{}", s);
    std::fflush(stdout);

    return 0;
}
