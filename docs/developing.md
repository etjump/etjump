# Development

ETJump is developed using `c++`, we prefer to have separate files for each standalone module for better isolation. Follow the [styleguide](styleguide.md) when formatting the code.

After the cmake configuration build/etjump folder will have the copy of the assets directory.
In case new asset file is added, you would need to rerun the cmake configuration again(`cmake .`).

## Debugging

### Linux

Using gdb. Make sure to compile mod as `Debug` build type.

```sh
gdb --args etl +set fs_homepath . +set fs_game etjump
# opens up gdb cli
r # to run the engine
```

### Windows

<!-- TODO. Add proper visual studio debugging setup example, for both `sln` and cmake based project. -->

1. For `sln`, select `cgame` as startup project.
2. Open up `cgame` project properties.
3. Open `Debug` tab.
4. Select `attach to executable` type of debug.
5. Select the `startup executable` and `Browse` game executable.
6. Select working directory as game directory of game executable.
7. Put in next arguments: `+set fs_homepath $(SolutionDir) +set fs_game etjump`

You can then launch the game from within the Visual Studio itself by clicking the `Play` button.
