# Compiling

Compiling the source code is a straightforward process. Below you will find instructions for each supported environment.

* __[Linux](#linux)__
    * [Makefiles](#option-1-makefiles) (Recommended)
    * [QtCreator](#option-2-qtcreator)
* __[Windows](#windows)__
    * [Visual Studio solution](#option-1-generate-visual-studio-sln-project-files) (Recommended)
    * [Visual Studio CMake project](#option-2-open-cmake-project-in-visual-studio)
    * [mingw-w64](#option-3-use-mingw-w64-toolchain)
    * [Cross-compiling windows binaries on linux using mingw-w64](#option-4-cross-compiling-windows-binaries-using-mingw-w64-on-linux)
* __[macOS](#macos)__
    * [Makefiles](#option-1-makefiles-and-apple-clang)
    * [Cross-compiling macOS binaries on linux using darling](#option-2-cross-compiling-macOS-binaries-on-linux-using-darling)

## Prerequisites

To compile etjump source code, you only need a few things:
* [Git](https://git-scm.com/) or [Git Desktop](https://desktop.github.com/) to fetch the repository. 
* [CMake](https://cmake.org/) to generate build files.
* [Visual Studio](https://visualstudio.microsoft.com/vs/community/) or `gcc` to compile the code.

## Building 

ETJump currently can be built only on `Windows` and `Linux` platforms. Supported build types are `Release` and `Debug`.

### Linux

There are multiple options available:
* Compile project using `Makefiles`.
* Use `QtCreator` IDE.

#### Option 1. Makefiles

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

##### Create mod pk3

* Run `make mod_pk3` to create mod pk3
    * `pk3` file will be created in `build/etjump` directory
* Run `make mod_release` to create zip release
    * `zip` release will be created in `build` directory

#### Option 2. QtCreator

[QtCreator](https://www.qt.io/product/development-tools) is a free C/C++ IDE. A full featured `Visual Studio` alternative on linux.

* Installation:
    ```sh
    # on ubuntu
    sudo apt-get -y install openjdk-7-jre qtcreator build-essential
    # on arch 
    sudo pacman -S qtcreator
    ```
* Fetch ETJump:
    ```sh
    $ git clone https://github.com/etjump/etjump.git
    # as a result you will have etjump directory containing source files
    ```
* Open up `CMakeLists.txt` (located in the root directory) in the editor and build the project.  

_TODO: work out the steps_

### Windows

There are multiple options available:
* Generate the `sln` project files (native `Visual Studio` experience). `RECOMMENDED`
* Load cmake project in [Visual Studio](https://visualstudio.microsoft.com/vs/community/) (`Visual Studio 2017` or later).
* Compiling using `mingw-w64` toolchain (using `gcc` on windows).
* Cross-compiling windows binaries on linux using `mingw-w64` toolchain.

#### Option 1. Generate Visual Studio `sln` project files:

This option gives best control over the solution and provides familiar usage experience. It is hence __recommended__ option for the mod [development and debugging](developing.md). 

##### CLI

1. Generate project files: 
    ```sh
    $ git clone https://github.com/etjump/etjump.git && cd etjump
    $ mkdir build && cd build
    $ cmake .. -G "Visual Studio 17 2022" -A Win32 # adjust if necessary
    ```
    * Replace the Visual Studio version with the version you have installed, use `cmake --help` or see [CMake documetation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators).
    * 64-bit architecture is also supported on Windows, and can be generated with `-A x64`. Note that ET 2.60b does not support 64-bit mods on Windows, and you need to debug and run the game with a 64-bit engine, such as [ET: Legacy](https://www.etlegacy.com/) or [ETe](https://github.com/etfdevs/ETe).

2. Open up generated `sln` solution (located in `build` directory) in `Visual Studio`. 
3. Use `Build` > `Build Solution` option to start compilation process.
4. Alternatively build directly from the command line:
    ```sh
    $ cmake --build . --config Release
    # or "cmake --build . --config Debug" to compile debuggable builds
    ```

##### CMake GUI

1. Fetch etjump project either using `Git Desktop` or `git cli`:
    ```sh
    cd Documents/Git
    $ git clone https://github.com/etjump/etjump.git
    # as a result you will have etjump directory containing source files
    ```
2. Open up `CMake GUI`.
3. For the `Where is the source code` select __etjump__ source directory.
4. For the `Where to build` create `build` directory within the __etjump__ source directory and select it.
5. Press `Configure` button.
6. For the `Specify generator` select the `Visual Studio` version you have installed (eg. `Visual Studio 2019`).
7. For the `Optional platform` select `Win32`.
8. Press okay and let it to configure.
9. Press `Generate` button.
10. Press `Open Project` button to open project in the `Visual Studio`.    

You can close cmake now.
* Press `Build > Build Solution` to start the compilation.
* You will find files in the `build/etjump` directory.
* Checkout the [Development and Debugging](developing.md) for more information on how to run and debug etjump.

##### Create mod pk3

In `Solution Explorer` (File list):
1. Select `Package/mod_pk3` project.
2. Open context menu and click on `Build` entry.
    * `pk3` file will be created in `build/etjump` directory.
3. Similarly you can create zipped etjump release by building `mod_release` project.
    * `zip` release will be created in `build` directory.

#### Option 2. Open CMake project in Visual Studio

This is simple, and easy use option, but not as much versatile as `sln` solution. This option requires `Visual Studio 2017` or later. Basically, you are able to open up `CMakeLists.txt` natively in the IDE to compile and edit the source code, without the need to run cmake generator manually.

1. Open up console and head to directory where you want to clone the `etjump` repository into (eg. `Documents/Git`)
2. Clone the repository
    ```sh
    $ git clone https://github.com/etjump/etjump.git 
    # as a result you will have etjump directory containing source files
    ```
3. Open up `Visual Studio`.
4. If `Start Window` is opened, press `Continue without code`.
5. In `File` > `Open`  select `CMake`.
6. Head to directory that contains cloned repository (eg. `Documents/Git/etjump`)
7. Select the `CMakeLists.txt` file.
8. Let it fully __initialize__ (output window should log `CMake generation finished.`).
9. In the `Build` menu select `Build All`.
    * Alternatively click on `Switch View` in `Solution Explorer`, switch to `CMake Targets View` and compile modules (`cgame`, `qagame`, `ui`) separately using context menu (requires `Visual Studio 2019` and later).
10. You will find compiled binaries in `build/etjump` directory. 

##### Create mod pk3

###### Visual Studio 2019

In `Solution Explorer` (File list):
1. Click `Switch Views` (4th button on the icon row).
2. Switch to `CMake Targets View`.
3. Select `Package/mod_pk3` entry.
4. Draw context menu and press `Build`.
5. You will find `pk3` file in `build/etjump` directory.
7. Similarly you can zip `pk3` and `qagame` module using `Package/mod_release` target.

###### Visual Studio 2017
* `CMake` menu entry should allow to run `mod_pk3` target.

#### Option 3. Use mingw-w64 toolchain

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

#### Option 4. Cross-compiling windows binaries on linux using `mingw-w64` 

As prerequisites you would first need to install `mingw-w64` and all its dependencies on your system using package manager.

```sh
# on ubuntu
sudo apt-get install -y mingw-w64
```

_FIXME: on ubuntu `mingw-w64` doesn't support std threads atm, hence failing to compile, use __arch__ instead_

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

#### Option 1. Makefiles and apple clang

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

#### Option 2. Cross-compiling macOS binaries on linux using `darling`

* Build darling from the source code followig the [instructions](https://docs.darlinghq.org/build-instructions.html) for the platform.
* Launch the [darling shell](https://docs.darlinghq.org/darling-shell.html).
* Run the commands from the previous chapter (Option 1).

## Building complete release package on linux with cross compile option (windows binaries)

Having all necessary packages installed (`gcc`, `gcc-multilib` and `mingw-w64`), one can build complete release on a single platform (`linux`), running the `build-release.sh` script file.

* Compile `x86` and `x86_64` linux release binaries and create a `zip` release package:
    ```sh
    $ mkdir build && cd build
    $ sh ../scripts/build-release.sh
    ```
* Or compile `x86` and `x86_64` linux + cross-compile __windows__ binaries (requires `mingw-w64`):
    ```sh
    $ mkdir build && cd build
    $ sh ../scripts/build-release.sh -mingw
    ```
* On success, you can find zip file in `build/etjump` directory.
