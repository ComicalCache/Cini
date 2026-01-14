# Cini

An "Emacs style" terminal-based text-editor.

> The name Cini is derived from [Mini](https://github.com/ComicalCache/Mini), my other terminal-based text-editor.
> Cini because it is written in C++, as opposed to Mini which is written in Rust, and because together they are now Cini
> Mini (don't sue me Nestlé, I'm broke).

## Configuration

Mini can be configured using [Lua](https://www.lua.org/docs.html). For an example of the API (and the complete default
configuration) checkout `lua/`. The entrypoints of the user configuration must be placed in
`$HOME/.config/cini/`.
- `init.lua` is loaded _before_ the editors internal state is set up and can be used to set editor global settings.
- `post_init.lua` is loaded _after_ the editors internal state is set up and can be used to set opened viewport/document
    specific settings.

## Build

Cini is built with CMake but provides a Makefile for convenience. Run `make help` to see the configure and build
commands.

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
