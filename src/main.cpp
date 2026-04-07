#include <csignal>
#include <print>

#include "cli_parser.hpp"
#include "editor.hpp"
#include "gen/lua_defaults.hpp"
#include "gen/version.hpp"
#include "util/ansi.hpp"
#include "util/fs.hpp"

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

    Editor::bootstrap();
    CliParser cli(argc, argv, Editor::instance()->lua_);

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
