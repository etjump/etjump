# Code style guide

*Note: the current codebase does not necessarily reflect the style outlined in this style guide. We did not have a proper style guide in the past. Any new code should follow this guide. Feel free to refactor old code to follow these conventions.*

## Code formatting

We use `clang-format` to keep the source code formatting consistent. Before committing your code, format it with `clang-format` using the formatting file provided in the repository.

```
clang-format -i <file1> <file2> ... <fileN>
``` 

Since the entire source is already formatted, it is okay to format the entire file you've worked on, rather than just the changes. You can however only format the code you've changed if you wish to. After adding files to commit, simply run `clang-format` with git.

```
git clang-format -i <file1> <file2> ... <fileN>
```

**Only format code inside the `src` directory!** We only format our own code. Do **NOT** run `clang-format` on dependencies, or asset files, such as shaders and menus.

## Assets formatting

* Use tabs for indentation when working on assets, such as shaders or menu files.
* Braces should always be put on separate lines when working with shaders.
* *Never run `clang-format` on assets!*

## Changelog formatting

We include changelog in the in-game menus. The [changelog.md](../changelog.md) file is parsed as a CMake configuration step into version-specific plaintext files, which are included in the mods assets. To ensure this parsing works as expected, there are few rules you should follow when adding content to the changelog.

* Only use `H1` headers (single `#`) and annotate each versions changelog separately with these - the parser uses `H1` headers to distinguish the versions.
* Start each line with a `*`, the changelog should consist of only bulletpoint list entries.
  * Multi-level lists like this are fine.
  * When indenting multi-level lists, use __2__ spaces instead of tabs.
* Markdown links are fine - the parser strips these out completely.

## Other files

We recognize it might be annoying to install bunch of auxiliary tools to simply format things, therefore you may consider installing and using these optional.

* YAML and JSON files should be formatted with [prettier](https://prettier.io/).
  * [`.editorconfig`](../.editorconfig) provides all the required settings for these
* Shell scripts should be formatted with [shfmt](https://github.com/patrickvane/shfmt).

## Coding conventions

In general, it's preferred to write more "C++ style" code rather than "C-style" code. This means for example using the C++ standard template library where applicable (e.g. use `std::array` over C-style array), C++ style casts instead of C-style casts and `nullptr` instead of `NULL`. It's perfectly fine to incorporate these types of changes to your commits when working on something: if you're fixing/modifying a function, feel free to replace any old usage of `NULL` within the function with `nullptr` for example. These conventions apply to the whole codebase; old, legacy code should be modernized as it is worked on.

## General guidelines

### Naming

* Functions, class methods and variables should be __camelCased__:

```cpp
void aCamelCasedName() {}
```

* `Class` names and `enums` should be __PascalCased__:

```cpp
class FileSystem {
  enum FileOpenModes {};
};
```

* Do not prefix private class members or methods with `_`, `m_`, or any other prefix.
* Compile-time constants (replacing magic numbers or strings etc, usually via `constexpr`) should be all uppercase, with underscores separating words. This doesn't apply to function-local `const` variables:

```cpp
// bad
inline constexpr float maxDist = 100.0f;

// good
inline constexpr float MAX_DIST = 100.0f;

void myFunc() {
  // good, function-local constant variable
  const float maxDist = 100.0f;
  // also good, constexpr constant inside function
  static constexpr float MAX_DIST = 100.0f;
}
```

### Files & headers

* Every new `.h`/`.cpp` file should include the full MIT license in the header. You can add this automatically by running [`scripts/include-license.sh`](../scripts/include-license.sh) (adds it to any `etj_*` source lacking one).
* Headers should use `#pragma once` rather than include guards.
* Header includes should be ordered as such, with a blank space between each group:
  * STL headers
  * module-specific headers
  * cross-module headers

```cpp
#include <algorithm>

#include "cg_local.h"
#include "etj_local.h"

#include "../game/etj_string_utilities.h"
```

### Code style

* Everything new (`ETJump` related) should be in a namespace; use `ETJump::` unless a more specific one fits (e.g. `DateTime::`):

```cpp
namespace ETJump {
class Example {};
}
```

* Prefer forward-declaring classes over including their headers when only a reference (pointer/reference/parameter type) is needed:

```cpp
namespace ETJump {
class CvarUpdateHandler;
class SnaphudData;

class CGazV2 : public IRenderable {
  // only needs a reference, so forward declaration is enough
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::shared_ptr<SnaphudData> snaphudData;
};
}
```

* Prefer const-correctness: mark parameters and locals `const`, pass non-trivial objects by `const&`, and make accessors `const`.
* Mark value-returning functions `[[nodiscard]]` when ignoring the result is a bug.
* Prefer `enum class` over plain `enum`; use explicit `static_cast` when converting to/from integer storage:

```cpp
enum class Style { FULL = 0, PERCENT = 1, NUMBER = 2 };

// converting from a cvar/integer requires an explicit cast
const Style style = static_cast<Style>(etj_strafeQualityStyle.integer);
```
* Prefer `auto` for obvious types (e.g. `const auto *const`, `const auto &`), but spell out the type when it aids readability.
* Avoid repeating type for variable declarations, when the result is assigned from a cast:

```cpp
// bad, value repeated in declaration and cast
float foo = static_cast<float>(getValue());

// good, declared as auto, value comes from cast
auto foo = static_cast<float>(getValue());
```

* In constructors, prefer member-initializer lists over assignments in the body, and `std::move` when storing by value:

```cpp
Foo(std::string name) : name(std::move(name)) { ... }
```

* Avoid narrowing conversions: prefer explicit casting over implicit conversions.
* Always initialize variables at declaration.
* Use `assert()` for programmer-error preconditions (null checks, impossible states):

```cpp
void Foo(gentity_t *ent) {
  assert(ent);   // callers must never pass null
  ...
}
```

* Prefer fixed-width integer types (`int32_t`, `uint32_t`) over `int`/`unsigned`.
* *Never* use `using namespace std`.

The repository's [`.clang-tidy`](../.clang-tidy) file also encodes a set of conventions. The following are reflected in how code is actually written:

* Avoid magic numbers and strings: prefer `inline`/`static constexpr` over `#define` macros for compile-time constants.
* Mark single-argument constructors `explicit` to avoid implicit conversions:

```cpp
class Wrapper {
public:
  explicit Wrapper(std::string value) : value(std::move(value)) {}
};
```

* Mark internal, file-local functions/variables `static` or use anonymous namespaces to force internal linkage:

```cpp
// only used within this translation unit
static bool isReady(const gentity_t *ent) { ... }

namespace {
struct MyStruct {
  ...
};
}
```

* Avoid recursive functions, unless it is the clearest way to express the logic.
* We don't really do runtime object destruction, copying or moving - an object is constructed on module init and destroyed on module shutdown the vast majority of the time. As such, the usual rule-of-five style obligations don't really apply - this is reflected by `cppcoreguidelines-special-member-functions.AllowSoleDefaultDtor: true` in the [`.clang-tidy`](../.clang-tidy) file.

Much of `.clang-tidy`'s strictness is deliberately disabled. The original code base is very old and contains substantial amounts of legacy C code that would require large refactors to satisfy those checks, so the relaxation is intentional, not an oversight - don't re-enable checks without good reason.

### Object ownership & lifetime

As mentioned above, objects should be constructed at module init, and destroyed at module shutdown the vast majority of the time. All objects of a module should be owned by a single "context" struct, to guarantee predictable construction & destruction order. This pattern is followed in `cgame` and `ui`, `game` is currently not following this correctly. See [`src/cgame/etj_cgame.h`](../src/cgame/etj_cgame.h) & [`src/cgame/etj_main.cpp`](../src/cgame/etj_main.cpp) for details.

* Use `std::unique_ptr` for objects owned by the context — i.e. everything with a single owner (renderables, utilities, etc.).
* Use `std::shared_ptr` only where an object is genuinely shared by multiple consumers, e.g. the core services (`cvarUpdate`, `consoleCommands`, `playerEvents`, ...) passed into many constructors. Because objects live for the whole process, the refcount overhead is a non-factor when it comes to performance.

### Comments

* Explain *why*, not what, in comments. Some code is by nature difficult to follow however - you may add more descriptive explanatory comments in such cases.

```cpp
// bad: restates the code
// check if the speed is greater than zero
if (speed > 0) { ... }

// good: explains the reason
// don't let interpolated frames modify jump times & sprint consumption
const bool isLerpFrame = s.pm.pmove_msec > cg.time - s.pm.cmd.serverTime;
```

* Mark known limitations and future work inline with `// TODO:` / `// FIXME:`.

### Control flow

* Prefer early exits (`return`, `continue`, `break`) over deeply nested conditionals; guard clauses that bail out early keep the happy path flat and makes the code more readable:

```cpp
// good: validate and bail out early
bool canSkipUpdate(const PmoveUtilsV2::State &s) {
  // not strafing
  if (!s.pm.cmd.forwardmove && !s.pm.cmd.rightmove) {
    return true;
  }

  // only air or slick movement is important
  if (s.pm.ps->groundEntityNum != ENTITYNUM_NONE &&
      !(s.pml.groundTrace.surfaceFlags & SURF_SLICK)) {
    return true;
  }

  ...
  return false;
}

// bad: deeply nested conditionals obscuring the happy path
bool canSkipUpdate(const PmoveUtilsV2::State &s) {
  if (s.pm.cmd.forwardmove || s.pm.cmd.rightmove) {
    if (s.pm.ps->groundEntityNum == ENTITYNUM_NONE ||
        (s.pml.groundTrace.surfaceFlags & SURF_SLICK)) {
      ...
    }
  }
}
```

* Do not nest ternary operators

```cpp
// bad
auto isValid = isAValid() ? true : isBValid() ? true : false;

// good
bool isValid;
if (isAValid() || isBValid()) {
  isValid = true;
} else {
  isValid = false;
}
```

### Structure

* Favor small, focused helper functions over long monolithic ones.
* Unused code should be completely removed instead of commented out, unless it is somehow relevant.
