# Making new mod derivative

* ETJump is an opensource project, which means you can freely use its codebase to create another mod or custom variation. 
* The process of deriving essentially is very simple: to rename the mod you would at the least need to edit root `CMakeLists.txt` - simply set new name in the `project()` directive. 
* This will replace the mod name in all necessary places. For instance it will use new mod directory to build the mod and pack files, in-game console will yield new mod name upon initialization, and other UIs that uses mod name directly (like scoreboard) will display new mod name. 
* You still will need to manually change the name in the menu files.
* In case of custom ETJump variation, make sure you prefix or suffix the mod name, to avoid any unnecessary name collisions.
* The versioning depends on `git tag`, hence define new tag before each release.