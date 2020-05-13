# Compiling

Compiling the source code is a straightforward process. Below you will find instructions for each supported environment.

## Prerequisites

You would need to get [Git](https://git-scm.com/) to fetch the repository and [CMake](https://cmake.org/) to generate build files. It is assumed you already have development tools installed.

## Dependencies

ETJump depends on 3 libraries: `SQLite3`, `Boost` and `Google Test`, which can be installed in your system using package manager of your preference, __but are not mandatory to have__, in case of absence (which is usually the case for Windows) necessary dependencies will be automatically fetched upon cmake configuration step. 

## Building 

Supported build types are `Release` and `Debug`.

### Linux

Generate Makefiles:

```sh
$ git clone https://github.com/etjump/etjump.git && cd etjump
$ mkdir build && cd build
# build native x86_64 (64bit):
$ cmake .. -DCMAKE_BUILD_TYPE=Release
# or build x86 (32bit) instead:
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-x86-linux.cmake
$ make -j4
# use this to create mod pk3
$ make mod_pk3
# files are compiled within the build directory in etjump folder
# you can then playtest the binaries using et or etl:
$ etl +set fs_homepath . +set fs_game etjump
```

### Windows

There are multiple options available:
* genereting the `sln` project files
* natively load the cmake project (Visual Studio 2017 and later)
* compiling using mingw64 toolchain

#### Generate Visul Studio `sln` project files:

```sh
$ git clone https://github.com/etjump/etjump.git && cd etjump
$ mkdir build && cd build
$ cmake .. -G "Visual Studio 15 2017" -A Win32
# you can then open up generated sln project in Visual Studio 
# or build directly from cli:
$ cmake --build . --config Release
```

#### Mingw-w64

```sh
$ git clone https://github.com/etjump/etjump.git && cd etjump
$ mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-mingw-x86-windows.cmake -G "MinGW Makefiles"
$ mingw32-make -j4
# create pk3
$ mingw32-make mod_pk3
```

### Cross-compiling windows binaries using `Mingw-w64` on linux

As prerequisites you would first need to install `mingw-w64-gcc` and all its dependencies on your system using package manager.

```sh
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-mingw-x86-linux.cmake
$ make -j4
```

## Building complete release package (linux) 

```sh
# compile release binaries and create a zip package
# compiles x86 and x86_64 linux binaries only
$ mkdir build && cd build
$ sh ../scripts/build-release.sh
# or linux + crosscompile windows binaries (needs mingw-w64)
$ sh ../scripts/build-release.sh -mingw
# after completion you will find zip file in build directory
```
