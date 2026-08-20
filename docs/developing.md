# Development

This section provides a helping guide on how to develop and debug ETJump.

ETJump is developed using `C++` (C++17), although a large portion of the code base is written in `C` (the original SDK code). You can work with the code in any editor or IDE you like; see the [IDEs](#ides) section for setup instructions for Visual Studio, VS Code, and CLion. It is assumed you know how to compile the code, otherwise check out the [compilation guide](compiling.md). It is also assumed you know your way around `C` and/or `C++`.

For running tests, check the [testing guide](testing.md).

The ETJump build system is based on [CMake](https://cmake.org/). While it's not really necessary to be an expert in this field, it would be desirable to at least get through the basics of [CMake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html).

For any questions, ETJump has its own [Discord](https://discord.gg/AcyWMqR) server, make sure to join it if you haven't yet.

## Main guidelines

* Prefer separate files for each standalone module for better isolation.
* Follow the [styleguide](styleguide.md) when formatting the code.
* Prefer writing tests for your code when possible.

## Project structure overview

* `assets` contains all static files ETJump uses at runtime. Things like menus, graphics, shaders and other scripts are stored here.
* `cmake` contains CMake scripts used during project generation, including toolchains for various platforms.
* `deps` contains all bundled dependencies ETJump uses under the hood. If you need to add a new dependency, this is the place for it. A glue `CMakeLists.txt` might need to be added in case a library doesn't come with one.
* `docs` contains all ETJump development related documentation.
* `scripts` contain auxiliary shell scripts.
* `src` is the code repository.
    * `cgame` contains the client side source code of the game.
        * Manages entity scene setup.
        * Draws HUDs and UIs.
        * Performs movement and weapon predictions.
        * Responds to game events.
    * `game` contains the server side source code of the game.
        * Manages entity lifecycles.
        * Processes world updates.
        * Defines game rules.
        * Performs physics and weapon calculations.
    * `ui` contains menu rendering related code.
* `tests` is the test repository. All test files are added here.

Note:
* After the CMake configuration, the `build/etjump` directory will contain a copy of the `assets` folder.
* If a new asset is added, you need to re-run the CMake configuration (simply run `cmake .` in the `build` directory).

## IDEs

ETJump is a CMake project, so any editor or IDE with CMake support can be used to build and debug it. Choose whichever suits you best. Building and configuring the project in each IDE is covered in the [compilation guide](compiling.md); debugging setup is covered in [Debugging](#debugging) below.

### Visual Studio

Visual Studio is the recommended IDE on Windows and provides the most complete debugging experience out of the box. See [Debugging with Visual Studio](#debugging-with-visual-studio) below.

### CLion

CLion has native CMake support and works with multiple toolchains (GCC, Clang, MSVC, MinGW) on Linux, Windows, and macOS. See [Debugging with CLion](#debugging-with-clion) below.

### Visual Studio Code

VS Code is a cross-platform editor that works on Linux, Windows, and macOS. It uses the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) extensions to configure and build the project. See [Debugging with VS Code](#debugging-with-vs-code) below.

## Running the mod

The mod is not a standalone executable - it is loaded by the game engine (ET, ETe, ET: Legacy) via command line arguments. The two key paths are:

* `fs_homepath` points to an ET installation directory, which contains the engine and `etmain` with the game assets (`pak0-2.pk3`, stock configs etc.).
* `fs_basepath` is the current development directory (e.g. `build`). The mod binaries are compiled here into the `etjump` directory, which mimics the path structure of an ET installation.

The game is launched with `+set fs_basepath . +set fs_homepath <install dir> +set fs_game etjump`. This launches the game executable from `fs_homepath`, which contains all the game assets, but loads the mod from `fs_basepath`, which contains the compiled mod binaries. Any files created at runtime by the game/mod will be created in `fs_homepath/etjump`, so any configs/custom maps you want to use during development should be placed inside `fs_homepath`.

On Linux, run the mod from a terminal with:

```sh
# replace engine/paths to match your specific environment
etl.x86_64 +set fs_basepath <path to build dir> +set fs_homepath <et install dir> +set fs_game etjump
```

On Windows:

```sh
# replace engine/paths to match your specific environment
C:\Games\ETDev\ET.exe +set fs_basepath C:\Dev\etjump\build +set fs_homepath C:\Games\ETDev +set fs_game etjump
```

When running from an IDE, set the **working directory** to the build directory so that `fs_basepath .` resolves to `build`, where the mod binaries live.

## Debugging

The general setup is the same across platforms: point the debugger at the game engine executable, pass the `+set fs_basepath . +set fs_homepath <install dir> +set fs_game etjump` arguments, set the working directory to the build directory, and place breakpoints in the mod source. When the game loads the mod, the breakpoints will be hit (e.g. set a breakpoint in `CG_Init` and load a map with `/devmap goldrush`).

To debug the server (`qagame`) module, just run a listen server on the client - no special setup is needed.

Make sure to build a `Debug` configuration first ([compiling.md](compiling.md)) for the best debugging experience.

### Debugging with Visual Studio

Visual Studio can configure the debugger either manually or automatically via CMake.

#### Option 1 - setup debugger manually

1. Right click `cgame` in Solution Explorer and select `Properties`.
2. Select `Debugging` tab.
3. For `Command` field browse game executable (this should be inside your desired `fs_homepath`).
4. For `Working Directory` field, use the macro `$(SolutionDir)`.
5. For `Command arguments`, make sure `fs_homepath` points to the correct directory, and define any other cvars you want to pass on init.

#### Option 2 - automatic setup with CMake

You can automatically configure the debugger to sensible defaults using CMake variables `ET_PATH` and `ET_EXE_NAME`.

* `ET_PATH` will be used as `fs_homepath`, and is where `Command` field looks for the engine to run
* `ET_EXE_NAME` is the executable being launched (ET.exe, etl.exe, ETe.exe etc.)

**IMPORTANT!!!** - because Visual Studio builds the project in parallel, ensure `mod_pk3` target is re-built after the initial build of the project. Otherwise, the mod pk3 might be incomplete and the game might fail to load the mod.

To ensure the mod pk3 is always up-to-date, you can add a post-build event to `cgame` which automates building the mod pk3 every time the project is built.

1. Right click `cgame` in Solution Explorer and select `Properties`.
2. Navigate to `Build Events` -> `Post-Build Events`.
3. Set the `Command Line` field to `cd $(SolutionDir) && cmake --build . --target mod_pk3`.

You can then press the green play button to launch the game in debugging mode.

#### Disable console window

1. Open up `cgame` project properties using context menu.
2. Select `Linker` tab.
3. Select `System` category.
4. Set `SubSystem` to `Not Set`.

#### CMake based project

If you loaded the project as a CMake project rather than an `sln`, the flow is the same but uses the `CMake Targets View` (requires Visual Studio 2019 or later) to build individual modules and run the `mod_pk3` target.

### Debugging with CLion

1. Set `cgame` as the project to run.
2. Select `All targets` to ensure all modules get built (unless you want to build everything manually).
3. Set the **executable** to the game engine (e.g. `etl.x86_64` on Linux, `ET.exe`/`etl.exe` on Windows).
4. Set the **program arguments** to `+set fs_basepath . +set fs_homepath <et install dir> +set fs_game etjump`.
5. Set the **working directory** to the build directory (so `fs_basepath .` resolves to `build`, where the mod binaries live).
6. Place breakpoints in the mod source; when the game loads the mod, they will be hit (e.g. set a breakpoint in `CG_Init` and load a map with `/devmap goldrush`).

### Debugging with VS Code

Install the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) extensions, open the repository root and let CMake Tools configure the project. Use the CMake Tools status bar to select the build variant and build target, then add a `.vscode/launch.json` entry to debug the mod (set `program` to the engine executable - e.g. `etl.x86_64` on Linux, `ET.exe`/`etl.exe` on Windows, the `etl`/`ET` binary on macOS):

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "ETJump",
      "type": "cppdbg",
      "request": "launch",
      "program": "<path to et executable>",
      "args": ["+set", "fs_basepath", ".", "+set", "fs_homepath", "<et install dir>", "+set", "fs_game", "etjump"],
      "cwd": "${workspaceFolder}/build"
    }
  ]
}
```

### Debugging with gdb (Linux)

`gdb` is a command line interface to inspect mod execution using the terminal, and is a good choice for a plain terminal workflow on Linux.

* Make sure to [compile](compiling.md) the mod as a `Debug` build first (`-DCMAKE_BUILD_TYPE=Debug`).
* Install gdb:
    ```sh
    # ubuntu
    sudo apt install gdb
    # arch
    sudo pacman -S gdb
    ```
* Using `gdb`:
    ```sh
    # start gdb
    gdb --args etl +set fs_homepath . +set fs_game etjump
    # set breakpoint:
    b CG_Init
    # or "b cg_main.c:3450"
    # run the game:
    r
    # load map:
    /devmap goldrush
    # upon cgame load breakpoint will be hit
    # use gdb commands to inspect environment
    # next line:
    n
    # print variable value:
    p clientNum
    # list current lines:
    l
    # continue execution upon next break point entrance
    c
    ```
* There are many more useful commands you can learn at [https://www.tutorialspoint.com/gnu_debugger/index.htm](https://www.tutorialspoint.com/gnu_debugger/index.htm).

## Adding new files to the project

When you add a new source file, it is not automatically part of the build - you must register it in the module's `CMakeLists.txt` and reload the project so the build system picks it up.

1. Create the file(s) in the module directory (`cgame`, `game`, `ui`).
2. Add the file name to the module-specific `CMakeLists.txt` source list (cross-directory shared sources go in the same list; test sources go in `tests/CMakeLists.txt`).
3. Reload the CMake project so the new file is picked up:
   * **Visual Studio** - the project should reconfigure when `CMakeLists.txt` changes; if not, build the `CMake/ZERO_CHECK` project or reopen the solution.
   * **CLion** - use *Reload CMake Project* (or *Reset Cache and Reload Project* if a fresh configure is needed).
   * **VS Code** - CMake Tools re-configures when `CMakeLists.txt` is saved; if it doesn't pick up the file, run the *CMake: Delete Cache and Reconfigure* command.
   * **Command line** - simply re-run `cmake -B build` (or any build, which triggers a reconfigure).
