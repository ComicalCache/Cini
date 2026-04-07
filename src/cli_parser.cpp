#include "cli_parser.hpp"

#include <sol/state.hpp>

CliParser::CliParser(int argc, char* argv[], sol::table table) { // NOLINT(modernize-avoid-c-arrays)
    this->options_ = std::move(table);

    for (auto idx{1}; idx < argc; idx += 1) {
        std::string_view arg = argv[idx];

        if (arg.starts_with("--")) {
            std::string_view kv = arg.substr(2);
            auto sep = kv.find('=');

            if (sep != std::string_view::npos) { // --key=value.
                std::string_view key = kv.substr(0, sep);
                std::string_view value = kv.substr(sep + 1);
                this->options_[key] = value;
            } else { // --key.
                this->options_[kv] = true;
            }
        } else {
            // First argument that doesn't start with "--" is the file path.
            this->file_ = std::filesystem::path(arg);
            break;
        }
    }
}
