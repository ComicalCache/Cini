#include <csignal>
#include <print>

#include "ansi.hpp"
#include "editor.hpp"
#include "lua_defaults.hpp"
#include "util.hpp"
#include "version.hpp"

void signal_handler(const int signum) {
    uv_tty_reset_mode();

    // Reraise to generate dump.
    std::signal(signum, SIG_DFL);
    std::raise(signum);
}

int main(const int argc, char* argv[]) {
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGILL, signal_handler);
    std::signal(SIGFPE, signal_handler);

    std::setlocale(LC_ALL, "");

    if (argc > 1) {
        if (std::strcmp(argv[1], "--version") == 0) {
            std::println("{} (v{} - {} [{}])", version::NAME, version::VERSION, version::BUILD_DATE,
                         version::BUILD_TYPE);
            return 0;
        } else if (std::strcmp(argv[1], "--defaults") == 0) {
            const auto base = std::filesystem::current_path() / "defaults";
            for (const auto& [module_name, content]: lua_modules::files) {
                std::string path_str = module_name;
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

                if (!util::write_file(full_path, content, std::ios::out | std::ios::trunc)) {
                    std::println("Failed to write to file '{}'", full_path.string());
                    return 1;
                }
            }

            std::println("Default config has been written to '{}'", base.string());
            return 0;
        }
    }

    std::string s{};
    ansi::alt_screen(s);
    std::println("{}", s);

    auto path = argc > 1 ? std::optional(std::filesystem::path(argv[1])) : std::nullopt;
    Editor{}.init_uv().init_lua().init_bridge().init_state(path).run();

    s.clear();
    ansi::main_screen(s);
    std::println("{}", s);

    return 0;
}
