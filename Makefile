MODE ?= Debug

ifeq ($(MODE), Release)
    BUILD_DIR := build
    CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release
else
    BUILD_DIR := debug-build
    CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Debug
endif

OS := $(shell uname -s)
ifeq ($(OS), Darwin)
    SDK_PATH := $(shell xcrun --show-sdk-path)
    TIDY_FLAGS := --extra-arg=-isysroot --extra-arg=$(SDK_PATH)
endif

.PHONY: all configure build clean format check help

all: build

# 1. Configure.
# Usage: `make configure [MODE=Debug]`.
# Usage: `make configure MODE=Release`.
configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

# 2. Build.
# Usage: `make build [MODE=Debug]`.
# Usage: `make build MODE=Release`.
build:
	cmake --build $(BUILD_DIR) --parallel

# 3. Clean.
clean:
	rm -rf build
	rm -rf debug-build

# 4. Format.
format:
	find src -name "*.cpp" -exec clang-format -i --sort-includes {} +

# 5. Run clang-tidy.
# Usage: `make build [MODE=Debug]`.
# Usage: `make build MODE=Release`.
check:
	find src -name "*.cpp" -exec clang-tidy -p $(BUILD_DIR) $(TIDY_FLAGS) --quiet {} +

help:
	@echo "Usage:"
	@echo "  make configure [MODE=Debug|Release]"
	@echo "    Configures the CMake project. This is mandatory before building"
	@echo
	@echo "  make [build]   [MODE=Debug|Release]"
	@echo "    Builds the project"
	@echo
	@echo "  make clean"
	@echo "    Removes build folders for Debug and Release"
	@echo
	@echo "  make format"
	@echo "    Runs clang-format on all source-files in src/"
	@echo
	@echo "  make check     [MODE=Debug|Release]"
	@echo "    Runs clang-tidy on all source-files in src/"
