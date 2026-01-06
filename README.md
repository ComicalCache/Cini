# Cini

An "Emacs style" terminal based text-editor.

> The name Cini is derived from [Mini](https://github.com/ComicalCache/Mini), my other terminal based text-editor.
> Cini because it is written in C++, as opposed to Mini which is written in Rust, and because together they are now Cini
> Mini (don't sue me Nestlé, I'm broke).

## Configuration

Mini can be configured using [Lua](https://www.lua.org/docs.html). For an example of the API (and the complete default
configuration) checkout `lua/`. The entrypoints of the user configuration must be placed in
`$HOME/.config/cini/`.
- `init.lua` is loaded _before_ the editors internal state is set up and can be used to set editor global settings.
- `post_init.lua` is loaded _after_ the editors internal state is set up and can be used to set opened viewport/document
    specific settings.

> For further details about the API, checkout `Editor::init_bridge()` in `src/editor.cpp` which defines the Lua
> bindings.

## Build

Cini can be built via CMake. `cd` into the projects root directory and execute the following commands to build
Cini. The resulting binary will be found at `./build/bin/cini`. Built libraries will be found in `./build/lib/`, but
serve no further purpose.

```
> cmake -S . -B build
[...]
> cmake --build build --parallel
[...]
```

### Debug Build

```
> cmake -S . -B debug-build -DCMAKE_BUILD_TYPE=Debug
[...]
> cmake --build debug-build --parallel
[...]
```

## Dependencies

> All dependencies should be pulled and built by CMake and don't require prior installation.

### LuaJIT

[LuaJIT](https://luajit.org) as Lua runtime.

### sol2

[sol2](https://github.com/ThePhD/sol2) to interface Lua.

### libuv

[libuv](https://github.com/libuv/libuv) for the event loop.
