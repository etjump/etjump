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
$ etl +set fs_homepath . +set fs_game etjump
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

You can test run binaries using terminal:
```sh
# playtest the binaries using ETLegacy directly from the build directory
$ C:\\Games\\ETLegacy\\etl.exe +set fs_basepath C:\\Games\\ETLegacy\\ +set fs_homepath . +set fs_game etjump +set sv_pure 0
# or for VET
$  C:\\Games\\WolfET\\ET.exe +set fs_basepath . +set fs_homepath "C:\\[Games]\\ETMAP\\" +set fs_game etjump +set sv_pure 0
```
Or set up debugger (instructions are below) and run the game directly from `Visual Studio`.

### Debugging

#### sln based project

* __Option 1.__  
    Edit debugger settings and specify ET/ETLegacy executable and working directory manually.
    1. Open up `cgame` project properties using context menu.
    2. Select `Debugging` tab.
    3. For `Command` field browse game executable (eg. `C:\Games\ETLegacy\etl.exe`).
    4. For `Working Directory` field browse game installation directory (eg. `C:\Games\ETLegacy`).
* __Option 2.__   
    Set `ETROOT` environmental variable to point to ET/ETLegacy installation (eg. `C:\Games\ETLegacy`).

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
