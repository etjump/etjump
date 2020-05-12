#!/bin/sh

CORES=$(grep -c ^processor /proc/cpuinfo)
BUILD_TYPE=Release
BUILD_MINGW=0

parse_cmdline()
{
	for var in "$@" 
	do
		if [ "$var" = "-release" ]; then
			BUILD_TYPE=Release
		elif [ "$var" = "-debug" ]; then
			BUILD_TYPE=Debug
		elif [ "$var" = "-mingw" ]; then
			BUILD_MINGW=1
		fi		
	done
}

build_linux()
{
	# build x86
	rm CMakeCache.txt
	cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-x86-linux.cmake
	make --no-print-directory -j$CORES
	# build x86_64
	rm CMakeCache.txt
	cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTS=OFF
	make --no-print-directory -j$CORES
}

build_mingw()
{
	# build x86
	rm CMakeCache.txt
	cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-mingw-x86-linux.cmake
	make --no-print-directory -j$CORES
	# build x86_64
	rm CMakeCache.txt
	cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-mingw-x86_64-linux.cmake
	make --no-print-directory -j$CORES
}

parse_cmdline $@
build_linux
if [ $BUILD_MINGW == 1 ]; then
	build_mingw
fi

# make release package
make mod_release --no-print-directory
