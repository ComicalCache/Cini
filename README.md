# Cini

An "Emacs style" terminal based text-editor.

> The name Cini is derived from [Mini](https://github.com/ComicalCache/Mini), my other terminal based text-editor.
  Cini because it is written in C++, as opposed to Mini which is written in Rust and because together they are now Cini
  Mini, like the cereal (please don't sue me Nestlé).

## Build

Cini can be built via CMake. `cd` into the projects root directory and execute the following commands to build
Cini. The resulting binary will be found at `./build/bin/cini`.
```
> cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
[...]
> cmake --build build --parallel
[...]
```
