# Cini

An "Emacs style" terminal based text-editor.

> The name Cini is derived from [Mini](https://github.com/ComicalCache/Mini), my other terminal based text-editor.
  Cini because it is written in C++, as opposed to Mini which is written in Rust and because together they are now Cini
  Mini, like the cereal (please don't sue me Nestlé).

## Build

Cini can be built via CMake. `cd` into the projects root directory and execute the following commands to build
Cini. The resulting binary will be found at `./build/bin/cini`. Built libraries will be found in `./build/lib/`, but
surce no further purpose.

```
> cmake -S . -B build
[...]
> cmake --build build --parallel
[...]
```

## Dependencies

### LuaJIT

[LuaJIT](https://luajit.org) as Lua runtime.

### sol2

[sol2](https://github.com/ThePhD/sol2) to interface Lua.

### libuv

[libuv](https://github.com/libuv/libuv) for the event loop.
