#!/bin/sh
CORES=$(grep -c ^processor /proc/cpuinfo)
# build x86
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-x86-linux.cmake
make --no-print-directory -j$CORES
# build x86_64
rm CMakeCache.txt
cmake .. -DCMAKE_BUILD_TYPE=Release
make --no-print-directory -j$CORES
# make release package
make mod_release --no-print-directory
