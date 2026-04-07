#ifndef CLI_PARSER_HPP_
#define CLI_PARSER_HPP_

#include <filesystem>
#include <optional>

#include <sol/table.hpp>

/// A simple command line argument parser.
struct CliParser {
public:
    std::optional<std::filesystem::path> file_{};
    sol::table options_{};

public:
    CliParser(int argc, char* argv[], sol::state& lua); // NOLINT(modernize-avoid-c-arrays)
};

#endif
