# Cini

A terminal-based text-editor.

> The name Cini is derived from [Mini](https://github.com/ComicalCache/Mini), my other terminal-based text-editor.
> Cini because it is written in C++, as opposed to Mini which is written in Rust, together they are now Cini Mini 
> (don't sue me Nestlé, I'm broke).

Cini is a highly scriptable and customizable text-editor written in C++ with a flexible Lua front-end. Similar to Emacs 
, Cini, the editor, is almost entirely implemented in the Lua front-end, while the C++ back-end offers the 
infrastructure necessary to do so. Cini's (text-)properties are a powerful system that allow for attaching user defined 
data to individual documents or document views, as well as adding text- or view-properties to text ranges of the 
document data. Cini's event system and hooks allow it do dynamically react to state changes, making Cini a modern 
adaptive editor. Cini features a plethora of in-built features, a few important ones are:
- Window splits
- A directory viewer (similar to Emacs' dired)
- Search & Replace

## Configuration

Mini can be configured using [Lua](https://www.lua.org/docs.html). For an example of the API checkout `lua/`.
`lua/bindings/` contains all bindings exposed by Cini. `lua/core/` contains the Core (std) implementation. The user
configuration must be placed in `$HOME/.config/cini/` and a `init.lua` file serves as entry point. It must expose two 
functions, `setup()` and `init()` that get called during initialization according to the specification in 
`lua/init.lua`.

Example:
```lua
local UserConfig = {}

function UserConfig.setup() 
    -- Setup code.
end

function UserConfig.init() 
    -- Init code.
end

return UserConfig
```

> Using `./cini --defaults` you can dump the default configuration into a `default/` director in the current directory.
> This includes `@meta` files that can be used by your IDE to help with autocomplete and other features.

## Build & Development

### TL;DR

`> make configure MODE=Release && make MODE=Release`

---

Cini is built with CMake but provides a Makefile for convenience. Run `make help` to see available commands. By default
a debug build is assumed. The output binary is found in `debug-build/bin/cini` or `build/bin/cini`. The build libraries
are found in `debug-build/lib/` or `build/lib/` and are _statically_ linked. Ensure a C++23 capable compiler is
installed (like clang++-20 or higher) and your default C++ compiler. Alternatively, if it isn't your default, modify
the `Makefile`'s configuration step to specify the compiler using `-DCMAKE_CXX_COMPILER` (e.g. 
`	cmake -DCMAKE_CXX_COMPILER="clang++-20" -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)`).

> Clipboard support might require dynamic linking of certain platform libraries.

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

### Clip

[Clip](https://github.com/dacap/clip) as cross platform clipboard.

> Clip requires `libx11-dev` or `libX11-devel` on Linux.
