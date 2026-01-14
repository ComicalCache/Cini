MODE ?= Debug

ifeq ($(MODE), Release)
    BUILD_DIR := build
    CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release
else
    BUILD_DIR := debug-build
    CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Debug
endif

.PHONY: all configure build clean format help

all: build

# 1. Configure.
# Usage: `make configure [MODE=Debug]` (Debug).
# Usage: `make configure MODE=Release` (Release).
configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

# 2. Build.
# Usage: `make build [MODE=Debug]` (Debug).
# Usage: `make build MODE=Release` (Release).
build:
	cmake --build $(BUILD_DIR) --parallel

# 3. Clean.
clean:
	rm -rf build
	rm -rf debug-build

# 4. Format.
format:
	find src -name "*.cpp" -exec clang-format -i --sort-includes {} +

help:
	@echo "Usage:"
	@echo "  make configure [MODE=Debug|Release]"
	@echo "  make [build]   [MODE=Debug|Release]"
	@echo "  make clean"
	@echo "  make format"
