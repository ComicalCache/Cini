# Cini

An "Emacs style" terminal-based text-editor.

> The name Cini is derived from [Mini](https://github.com/ComicalCache/Mini), my other terminal-based text-editor.
> Cini because it is written in C++, as opposed to Mini which is written in Rust, and because together they are now Cini
> Mini (don't sue me Nestlé, I'm broke).

## Configuration

Mini can be configured using [Lua](https://www.lua.org/docs.html). For an example of the API checkout `lua/`.
`lua/bindings/` contains all bindings exposed by Cini. `lua/core/` contains the Core (std) implementation. The user
configuration must be placed in `$HOME/.config/cini/` and a `init.lua` file serves as entry point.

> Using `./cini --defaults` you can dump the default configuration into a `default/` director in the current directory.

## Build & Development

### TL;DR

`> make configure MODE=Release && make MODE=Release`

---

Cini is built with CMake but provides a Makefile for convenience. Run `make help` to see available commands. By default
a debug build is assumed. The output binary is found in `debug-build/bin/cini` or `build/bin/cini`. The build libraries
are found in `debug-build/lib/` or `build/lib/` and are _statically_ linked.

> `clang-tidy`, `clangd` and `clang-format` are used for static analysis and formatting.

## Dependencies

> All dependencies should be pulled and built by CMake and don't require prior installation.

### Lua

[Lua](https://lua.org) as the scripting language.

### sol2

[sol2](https://github.com/ThePhD/sol2) to interface Lua.

### libuv

[libuv](https://github.com/libuv/libuv) for the event loop.

### PCRE2

[PCRE2](https://github.com/PCRE2Project/pcre2) as Regex engine.
