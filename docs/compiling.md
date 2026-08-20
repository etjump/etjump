# Compiling

Compiling the source code is a straightforward process. Below you will find instructions for each supported environment.

* __[Linux](#linux)__
    * [CLion](#linux-clion) (Recommended)
    * [Visual Studio Code](#linux-visual-studio-code)
    * [Makefiles + gdb](#makefiles-gdb)
* __[Windows](#windows)__
    * [Visual Studio](#visual-studio) (Recommended)
    * [CLion](#windows-clion) (Recommended)
    * [Visual Studio Code](#windows-visual-studio-code)
    * [mingw-w64](#use-mingw-w64-toolchain)
    * [Cross-compiling windows binaries on Linux using mingw-w64](#cross-compiling-windows-binaries-on-linux-using-mingw-w64)
* __[macOS](#macos)__
    * [CLion](#macos-clion) (Recommended)
    * [Visual Studio Code](#macos-visual-studio-code)
    * [Makefiles](#makefiles-and-apple-clang)
    * [Cross-compiling macOS binaries on Linux using darling](#cross-compiling-macos-binaries-on-linux-using-darling)

## Prerequisites

To compile etjump source code, you only need a few things:
* [Git](https://git-scm.com/) or [Git Desktop](https://desktop.github.com/) to fetch the repository. 
* [CMake](https://cmake.org/) to generate build files.
* A C++ compiler for your platform - supported compilers are GCC, Clang, MSVC and MinGW.

## Building 

ETJump can currently be built on `Windows`, `Linux`, and `macOS`. Supported build types are `Release`, `Debug` and `RelWithDebInfo`.

### Sanitizers

The build can be configured with [sanitizers](https://clang.llvm.org/docs/AddressSanitizer.html) to detect memory and undefined-behavior errors at runtime. These are controlled by CMake options:

* `USE_SANITIZERS` - enables address, leak and undefined behavior sanitizers together (the per-sanitizer options below are then forced on for non-MSVC compilers).
* `USE_ASAN` - address sanitizer. Supported on GCC, Clang and MSVC.
* `USE_LSAN` - leak sanitizer. Not supported on MSVC. Leak sanitizer is also enabled by default whenever ASAN is on.
* `USE_UBSAN` - undefined behavior sanitizer. Not supported on MSVC.

For example, to build with all sanitizers:

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DUSE_SANITIZERS=ON
cmake --build build --parallel
```

#### Additional notes:
* The game engine may fail to load the mod when using sanitizers. If this happens, you may need to preload the sanitizer libraries, before running the engine.
* Sanitizers are most useful with `Debug` (or `RelWithDebInfo` for profiling).
* Leak and undefined behavior sanitizers are only supported on GCC/Clang; enabling them on MSVC fails the configuration.
* Clang support is only tested on Linux.
* On Windows with MSVC, running an ASAN build outside Visual Studio requires the sanitizer runtime libraries on `PATH` (Visual Studio adds these automatically).
* On Linux with Clang, the ASAN executable links against the shared ASAN runtime (`-shared-libasan`).

### Linux

<a id="linux-clion"></a>
#### CLion

[CLion](https://www.jetbrains.com/clion/) is a cross-platform C/C++ IDE with native CMake support, and the recommended fully-featured IDE on Linux.

* Open the repository root in CLion — it will configure the project automatically (it uses its own build directory by default; you can point it at `build/` if you prefer).
* Set `cgame` as the project to run, and select `All targets` to ensure all modules get built.
* To run/debug the mod, set up a run configuration launching the game with the arguments `+set fs_basepath . +set fs_homepath <et install dir> +set fs_game etjump`, with the working directory set to `$CMakeCurrentGenerationDir$`. See [debugging with CLion](developing.md#debugging-with-clion) for more details.

<a id="linux-visual-studio-code"></a>
#### Visual Studio Code

VS Code is a cross-platform editor that uses the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) extensions to configure and build the project.

* Open the repository root in VS Code and let CMake Tools configure the project.
* Use the CMake Tools status bar to select the build variant and build target.
* See [Debugging with VS Code](developing.md#debugging-with-vs-code) in the development guide for setting up a debug launch configuration.

#### Makefiles + gdb

For a plain terminal workflow, compile the project with `Makefiles` (or `Ninja`) and debug with `gdb`.

* Generate __x86_64__ Makefiles:
    ```sh
    $ git clone https://github.com/etjump/etjump.git && cd etjump
    $ mkdir build && cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Release
    # or "-DCMAKE_BUILD_TYPE=Debug" to compile debug binaries 
    ```
* Or generate __x86_32__ Makefiles:  
    Make sure `gcc-multilib` is installed first.
    ```sh
    $ git clone https://github.com/etjump/etjump.git && cd etjump
    $ mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-x86-linux.cmake
    ```
* Compile:
    ```sh
    # -j4 = use 4 threads
    $ make -j4
    ```
* You can find binaries in `build/etjump`.

Alternatively, use Ninja as the generator (`-G Ninja`); the build command then becomes `ninja` (or `cmake --build . --parallel`).

* To debug, see [Debugging with gdb](developing.md#debugging-with-gdb-linux).

##### Create mod pk3

* Run `make mod_pk3` to create mod pk3
    * `pk3` file will be created in `build/etjump` directory
* Run `make mod_release` to create zip release
    * `zip` release will be created in `build` directory

### Windows

There are multiple options available:
* Use [Visual Studio](#visual-studio) (Recommended).
* Use [CLion](#windows-clion) (Recommended).
* Use [Visual Studio Code](#windows-visual-studio-code).
* Compiling using `mingw-w64` toolchain (using `gcc` on Windows).
* Cross-compiling Windows binaries on Linux using `mingw-w64` toolchain.

#### Visual Studio

Visual Studio offers a complete Windows development and debugging experience. Both Visual Studio and [CLion](#windows-clion) are recommended options on Windows - pick whichever you prefer.

Generate the `sln` solution:

```sh
$ git clone https://github.com/etjump/etjump.git && cd etjump
$ mkdir build && cd build
$ cmake .. -G "Visual Studio 17 2022" -A Win32 # adjust if necessary
```

* Replace the Visual Studio version with the version you have installed, use `cmake --help` or see [CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators).
* 64-bit architecture is also supported on Windows, and can be generated with `-A x64`. Note that ET 2.60b does not support 64-bit mods on Windows, and you need to debug and run the game with a 64-bit engine, such as [ET: Legacy](https://www.etlegacy.com/) or [ETe](https://github.com/etfdevs/ETe).
* CMake variable `ET_PATH` can be used to point CMake to your development installation - Visual Studio sets up debugging with some pre-defined parameters, and this path will be used as `fs_homepath` and the directory where the game is located.
* CMake variable `ET_EXE_NAME` can be used to set the engine that gets launched from `ET_PATH` to debug.

Open up the generated `sln` solution (located in `build` directory) in Visual Studio and use `Build` > `Build Solution` to compile. Alternatively, build directly from the command line:

```sh
$ cmake --build . --config Release
# or "cmake --build . --config Debug" to compile debuggable builds
```

##### Create mod pk3

In `Solution Explorer` (File list):
1. Select `Package/mod_pk3` project.
2. Open context menu and click on `Build` entry.
    * `pk3` file will be created in `build/etjump` directory.
3. Similarly you can create zipped etjump release by building `mod_release` project.
    * `zip` release will be created in `build` directory.

You can also load the project directly in Visual Studio as a CMake project (via `File > Open > CMake`, selecting the root `CMakeLists.txt`) instead of generating an `sln`. This uses the `CMake Targets View` to build individual modules and run the `mod_pk3` target, but requires `Visual Studio 2019` or later.

See [Debugging with Visual Studio](developing.md#debugging-with-visual-studio) in the development guide for setting up the debugger.

<a id="windows-clion"></a>
#### CLion

[CLion](https://www.jetbrains.com/clion/) is a cross-platform C/C++ IDE with native CMake support, and is a recommended option on Windows alongside Visual Studio.

* Open the repository root in CLion — it will configure the project automatically.
* CLion supports both the MinGW-w64 and Microsoft Visual Studio toolchains; pick whichever you have installed.
* Set `cgame` as the project to run, and select `All targets` to ensure all modules get built.
* To run/debug the mod, set up a run configuration launching the game with the arguments `+set fs_basepath . +set fs_homepath <et install dir> +set fs_game etjump`, with the working directory set to `$CMakeCurrentGenerationDir$`. See [debugging with CLion](developing.md#debugging-with-clion) for more details.

<a id="windows-visual-studio-code"></a>
#### Visual Studio Code

Visual Studio Code uses the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) extensions to configure and build the project.

* Open the repository root in VS Code and let CMake Tools configure the project.
* Use the CMake Tools status bar to select the build variant and build target.
* See [Debugging with VS Code](developing.md#debugging-with-vs-code) in the development guide for setting up a debug launch configuration.

#### Use mingw-w64 toolchain

`mingw-w64` is a build toolchain which allows users to build software using `gcc` compiler and linking against static `libc`, making software more or less consistent among platforms (eg. linux).
_One can also use `mingw-w64` on linux to cross-compile windows binaries (next section)._

1. Download `MinGW-W64-install.exe` from [https://sourceforge.net/projects/mingw-w64/files/](https://sourceforge.net/projects/mingw-w64/files/)
2. Select 
    * GCC version `8.1` or later
    * Architecture `i686`
    * Threads `posix`
    * Exception `dwarf`
3. Open up `mingw-w64` console (find `mingw-w64.bat` in the install directory)
4. Open up the directory where you want to clone the `etjump` repository into (eg. `Documents/Git`)
5. Run next commands:
    ```sh
    $ git clone https://github.com/etjump/etjump.git && cd etjump
    $ mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-mingw-x86-windows.cmake -G "MinGW Makefiles"
    # -j4 = use 4 threads
    $ mingw32-make -j4
    # create pk3 
    $ mingw32-make mod_pk3
    # or use "mingw32-make mod_release" to create release zip
    ```

#### Cross-compiling windows binaries on linux using `mingw-w64` 

As prerequisites you would first need to install `mingw-w64` and all its dependencies on your system using package manager.

```sh
# on ubuntu
sudo apt-get install -y mingw-w64
```


```sh
# on arch
sudo pacman -S mingw-w64
```

Build the binaries:

```sh
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cross-mingw-x86-linux.cmake
# -j4 = use 4 threads
$ make -j4
```

### macOS

<a id="macos-clion"></a>
#### CLion

[CLion](https://www.jetbrains.com/clion/) is a cross-platform C/C++ IDE with native CMake support, and the recommended IDE on macOS (using the Apple Clang toolchain).

* Open the repository root in CLion — it will configure the project automatically (it uses its own build directory by default; you can point it at `build/` if you prefer).
* Set `cgame` as the project to run, and select `All targets` to ensure all modules get built.
* To run/debug the mod, set up a run configuration launching the game with the arguments `+set fs_basepath . +set fs_homepath <et install dir> +set fs_game etjump`, with the working directory set to `$CMakeCurrentGenerationDir$`. See [debugging with CLion](developing.md#debugging-with-clion) for more details.

<a id="macos-visual-studio-code"></a>
#### Visual Studio Code

VS Code is a cross-platform editor that uses the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) extensions to configure and build the project.

* Open the repository root in VS Code and let CMake Tools configure the project.
* Use the CMake Tools status bar to select the build variant and build target.
* See [Debugging with VS Code](developing.md#debugging-with-vs-code) in the development guide for setting up a debug launch configuration.

#### Makefiles and apple clang

For a plain terminal workflow, install the Apple developer tools and compile with `clang`:

* Install `clang` and all necessary gnu tools:
    ```sh
    xcode-select --install
    ```

* Install brew (this should also install `clang` if missing):
    ```sh
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    ```

* Install `cmake` and `git`:
    ```sh
    brew install cmake git
    ```

* Generate __x86_64__ Makefiles:
    ```sh
    $ git clone https://github.com/etjump/etjump.git && cd etjump
    $ mkdir build && cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Release
    # or "-DCMAKE_BUILD_TYPE=Debug" to compile debug binaries 
    ```

* Compile:
    ```sh
    # -j4 = use 4 threads
    $ make -j4
    ```
* You can find binaries in `build/etjump`.

##### Create mod pk3

* Run `make mod_pk3` to create mod pk3
    * `pk3` file will be created in `build/etjump` directory
* Run `make mod_release` to create zip release
    * `zip` release will be created in `build` directory

#### Cross-compiling macOS binaries on linux using `darling`

* Build darling from the source code following the [instructions](https://docs.darlinghq.org/build-instructions.html) for the platform.
* Launch the [darling shell](https://docs.darlinghq.org/darling-shell.html).
* Run the commands from the previous chapter (Option 3).
