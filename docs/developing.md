# Development 

This section provides a helping guide on how to develop and debug ETJump.

ETJump is developed using `C++`, although most of the code base is written in `C`. Essentially, you can work with code in any preferred editor, it is however recommended to use [Visual Studio](https://visualstudio.microsoft.com/vs/community/). It is assumed you know how to compile the code, else check out the [compilation guide](compiling.md). It is also assumed you know your way around `C` and/or `C++` languages. If not, it is recommended to read [C beginners tutorial](https://www.tutorialspoint.com/cprogramming/index.htm) and [C++ tutorial](https://www.learncpp.com/). The codebase is fairly straightforward and simple, so basic `C` knowledge would be sufficient to develop the mod.

For running tests check [testing guide](testing.md)

ETJump build system is based on [CMake](https://cmake.org/). While it's not really necessary to be an expert in this field, it would be desirable at least to get through basics of [cmake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html). 

For any questions, ETJump has own [discord](https://discord.gg/AcyWMqR) server, make sure to join it, if you haven't yet. ETJump documentation can be found [here](http://etjump.readthedocs.io/en/latest/). 

Everyone are welcome to push edits to the development guide to make it even more informative.

## Main guidelines

* Prefer to have separate files for each standalone module for better isolation. 
* Follow the [styleguide](styleguide.md) when formatting the code.
* Prefer writing tests for your code if possible.

## Project structure overview 

* `assets` contains all static files ETJump is using in runtime. Things like menus, graphics, shaders and other scripts, are stored here. 
* `cmake` contains `CMake` scripts that are used during project generation, including toolchains for various platforms.
* `deps` contains all bundled dependencies ETJump is using under the hood (gtest, sqlite, sha1, json, boost). In case you need to add new dependency, this should be the place for it. A glue `CMakeLists.txt` might need to be added in case library doesn't come with one.
* `docs` contains all ETJump development related manuals.
* `scripts` contain auxiliary bash scripts.
* `src` the code repository.
    * `cgame` contains client side source code of the game. 
        * Manages entity scene setup.
        * Draws HUDs and UIs.
        * Performs movement and weapon predictions. 
        * Responds on game events.
    * `game` contains server side source code of the game.
        * Manages entity lifecycles.
        * Process world updates.
        * Defines game rules.
        * Performs physics and weapon calculations.
    * `ui` contains menu rendering related code.
* `tests` tests repository. All test files are added here.

Note:
* After the cmake configuration, `build/etjump` directory will contain the copy of the `assets` folder.
* In case new asset is added, you would need to rerun the cmake configuration again (simply run `cmake .` in the `build` directory).

## Linux

### Running

```sh
# playtest the binaries using et or etl directly from the build directory
# replace engine/paths to match your specific environment
$ etl.x86_64 +set fs_basepath ~/dev/etjump/build +set fs_homepath ~/games/et-dev +set fs_game etjump
```

### Debugging

On linux you can use:
* `gdb` a command line interface to inspect mod execution using terminal. 
* `QtCreator` a `Visual Studio` like IDE with a GUI debugger.

#### GDB

* Make sure to [compile](compiling.md) mod as `Debug` build type first (`-DCMAKE_BUILD_TYPE=Debug`).
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

#### QtCreator

_TODO_ 

## Windows

### Running

You can test run the binaries using terminal directly:
```sh
# playtest the binaries directly from the build directory
# replace engine/paths to match your specific environment
$ C:\Games\ETDev\ET.exe +set fs_basepath C:\Dev\etjump\build +set fs_homepath C:\Games\ETDev +set fs_game etjump
```
Or set up debugger (instructions are below) and run the game directly from `Visual Studio`.

### Debugging

The general path setup for debugging and developing should be following:

* `fs_homepath` points to an ET installation directory, which contains the engine and `etmain` with the game assets (`pak0-2.pk3`, stock configs etc.)
* `fs_basepath` is the current development directory (e.g. `build`). The mod binaries are compiled here into `etjump` directory, which mimics the path structure of an ET installation.
* Working directory should be set to the same as `fs_basepath` (Visual Studio users can use the `$(SolutionDir)` macro for this).
* The game is launched with `+set fs_basepath . +set fs_homepath <path\to\install\dir> +set fs_game etjump`. This launches the game executable from `fs_homepath`, which contains all the game assets, but loads the mod from `fs_basepath`, which contains our compiled mod binaries. Any files created during runtime by the game/mod will be created in `fs_homepath/etjump`. Additionally, any configs/custom maps you might want to use during development should be placed inside `fs_homepath`.

#### sln based project

##### Option 1 - setup debugger manually

1. Right click `cgame` in Solution Explorer and select `Properties`.
2. Select `Debugging` tab.
3. For `Command` field browse game executable (this should be inside your desired `fs_homepath`).
4. For `Working Directory` field, use the macro `$(SolutionDir)`.
5. For `Command arguments`, make sure `fs_homepath` points to the correct directory, and define any other cvars you want to pass on init.

##### Option 2 - automatic setup with CMake

You can automatically configure the debugger to sensible defaults using CMake variables `ET_PATH` and `ET_EXE_NAME`.

* `ET_PATH` will be used as `fs_homepath`, and is where `Command` field looks for the engine to run
* `ET_EXE_NAME` is the executable being launched (ET.exe, etl.exe, ETe.exe etc.)

**IMPORTANT!!!** - because Visual Studio builds the project in parallel, ensure `mod_pk3` target is re-built after the initial build of the project. Otherwise, the mod pk3 might be incomplete and the game might fail to load the mod.

To ensure the mod pk3 is always up-to-date, you can add a post-build event to `cgame` which automates building the mod pk3 every time the project is built.

1. Right click `cgame` in Solution Explorer and select `Properties`.
2. Navigate to `Build Events` -> `Post-Build Events`.
3. Set the `Command Line` field to `cd $(SolutionDir) && cmake --build . --target mod_pk3`.

You can then press the green play button to launch the game in debugging mode.

##### Disable console window

1. Open up `cgame` project properties using context menu.
2. Select `Linker` tab.
3. Select `System` category.
4. Set `SubSystem` to `Not Set`.

#### CMake based project

_TODO_

### Adding new files to solution (`sln`)

1. Create file in the module directory (`cgame`, `game`, `ui`).
2. Add file name in the modules specific `CMakeLists.txt`.
3. Either reopen `Visual Studio` or build `CMake/ZERO_CHECK` project to make new file to appear in the `Solution View`. 
