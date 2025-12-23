#!/usr/bin/env bash
set -e

BUILD_DIR=${1:-"build"}
COMPILER=${2:-"cc"}

VENDOR_DIR="$BUILD_DIR/_deps/luajit-src"
INSTALL_LIB="$BUILD_DIR/lib"
INSTALL_INC="$BUILD_DIR/include/luajit"

if [ ! -d "$VENDOR_DIR" ]; then
    echo "Cloning into 'luajit-src'..."
    mkdir -p "$VENDOR_DIR"
    git clone https://luajit.org/git/luajit.git "$VENDOR_DIR"
fi

cd "$VENDOR_DIR"
if [ -f "$INSTALL_LIB/libluajit-5.1.a" ]; then
    exit 0
fi

if [[ "$(uname -s)" = "Darwin" ]]; then
    export MACOSX_DEPLOYMENT_TARGET=15.0
fi

make clean
make -j6 \
    CC="$COMPILER" \
    XCFLAGS="-fPIC -flto -march=native" \
    BUILDMODE=static

mkdir -p "$INSTALL_LIB"
mkdir -p "$INSTALL_INC"

cp src/libluajit.a "$INSTALL_LIB/libluajit-5.1.a"
cp src/lua.h src/lualib.h src/lauxlib.h src/luaconf.h src/lua.hpp src/luajit.h "$INSTALL_INC"

echo "Built target liblua"
