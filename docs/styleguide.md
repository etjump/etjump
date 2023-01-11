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
* Tab width should be set to **4** spaces.
* Braces should always be put on separate lines when working with shaders.
* *Never run `clang-format` on assets!*

## Coding conventions

In general, it's preferred to write more "C++ style" code rather than "C-style" code. This means for example using the C++ standard template library where applicable (e.g. use `std::array` over C-style array), C++ style casts instead of C-style casts and `nullptr` instead of `NULL`. It's perfectly fine to incorporate these types of changes to your commits when working on something: if you're fixing/modifying a function, feel free to replace any old usage of `NULL` within the function with `nullptr` for example.

## General guidelines

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

* Everything new (`ETJump` related) should be in a namespace. Use `ETJump::` if more appropriate is not available. (e.g. `DateTime::` for date utilities):

```cpp
namespace ETJump {
class Example {};
}; // namespace ETJump
```

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

* Class members and methods should be declared in the following order:
  * private (it is okay to omit the keyword)
  * protected
  * public
* Do not prefix private class members or methods with `_` or `m_` or any other prefix.
* Unused code should be completely removed instead of commented out, unless it is somehow relevant.
* Avoid narrowing conversions: prefer explicit casting rather than implicit.
* *Never* use `using namespace std`.
