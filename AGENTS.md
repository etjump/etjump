# AGENTS.md

ETJump is a C/C++ (C++17) mod for Wolfenstein: Enemy Territory. CMake build, single repo, no monorepo. Built on Linux/Windows/macOS; produces `cgame`, `qagame`, `ui` engine modules plus a packaged `.pk3`.

## Build & test

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # or Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure        # all tests
./build/tests/tests --gtest_filter='TimerunSharedTests.*'  # single suite / test
```

- Binaries land in `build/etjump/` (e.g. `cgame.mp.x86_64.so`, `qagame.mp.x86_64.so`, `etjump-<ver>.pk3`).
- Packaging targets: `cmake --build build --target mod_pk3` (pk3), `mod_release` (release zip). Release zips only include `qagame*` + pk3, not cgame/ui.
- `ctest` runs the single `tests` executable; CI runs `ctest` per config on Windows (`ctest -C Release`).
- CMake options: `BUILD_TESTS` (ON by default), `USE_SANITIZERS` (enables ASAN+LSAN+UBSAN) and per-sanitizer `USE_ASAN`/`USE_LSAN`/`USE_UBSAN`, `USE_CLANG`. Cross-compile toolchains live in `cmake/Toolchain-*.cmake` (x86 linux, mingw x86/x64 linux/windows).
- Do not add deps via system packages; vendored deps live in `deps/` and are wired up in the root `CMakeLists.txt`.

## Configure-time generated files (source tree gets modified!)

Running CMake **writes files into the source tree**, not just the build dir:
- `assets/ui/git_version.h` — from `git describe` (falls back to `VERSION.txt` when not a git repo / no tags).
- `assets/ui/changelog/version_headers.h` + one `assets/ui/changelog/<version>.txt` per H1 section — parsed from `changelog.md`.
- Version comes from `VERSION.txt` (`VERSION_MAJOR/MINOR/PATCH <n>` format, regex-parsed, keep the format) but `GAME_VERSION` string uses `git describe`, so releases need a git tag.

Implications: after adding assets, editing `changelog.md`, or changing `VERSION.txt`, re-run `cmake -B build` before building. These files are gitignored and regenerated on every configure, so there's nothing to commit or manually sync.

## changelog.md format (parser is fragile)

Parsed at configure time by `cmake/ParseChangelog.cmake`; malformed input breaks the build.
- One `#` H1 header per version; parser splits sections on H1.
- Bullet-list entries only; nested lists indented with 2 spaces (not tabs). Markdown links get stripped.

## Formatting & style

- Format with `clang-format -i` (repo `.clang-format`). **Only run it on code in `src/`** — never on `deps/` or `assets/` (shaders/menus). `git clang-format -i` is fine.
- Style guide (`docs/styleguide.md`): new code should be modern C++ — `ETJump::` namespaces (or more specific), camelCase functions/vars, PascalCase classes/enums, uppercase `constexpr` constants, no `using namespace std`, no `_`/`m_` member prefixes, no nested ternaries, early exits over deep nesting, `enum class` + explicit casts, `[[nodiscard]]` where applicable, fixed-width int types. Old C-style code is exempt; don't refactor unrelated files. The `.clang-tidy` relaxations are deliberate (legacy C), not to be re-enabled casually.
- Objects are constructed at module init and destroyed at shutdown, owned by a single per-module context struct (see `src/cgame/etj_cgame.h`); `shared_ptr` only for shared services/data, `unique_ptr` for sole ownership, raw refs for observers. `game` still needs migrating to this.

## Tests

- GoogleTest. Add new tests in `tests/` and register both the test file and any new source it needs in `tests/CMakeLists.txt` (`add_executable(tests ...)` list) — the build links individual `src/` translation units, not whole modules.
- Test files include `src/game/etj_*.cpp` and `src/cgame/etj_*.cpp` sources directly; the game engine (q_shared, bg_*, etc.) is not linked, so tests are for self-contained modules only.
- The repo contains no game engine to run, so gameplay/rendering changes cannot be verified by automated tests — they require interactive playtesting (see `docs/developing.md` for running the mod). Unit tests only cover self-contained logic; don't try to verify game-specific behavior via ctest.

## Code layout

- `src/game/` — server (qagame): entities, physics, cvars/commands, database/save systems. `src/cgame/` — client: drawing, prediction, HUD. `src/ui/` — menu rendering. All of this is mod game-code (no engine lives here); `cg_main.cpp`/`g_main.cpp`/`ui_main.cpp` are the `vmMain` entry points and `*_syscalls.cpp` wrap engine API calls. Everything is free to edit except a few API-sensitive things (e.g. networked structs).
- `etj_*.cpp/h` files are the newer, ETJump-specific modules; `bg_*`, `cg_*`, `g_*`, `q_*` are original SDK code that is still actively edited.
- CMake: module sources are listed explicitly in each module's `CMakeLists.txt` (`src/game`, `src/cgame`, `src/ui`) — adding a new `.cpp`/`.h` requires registering it in the relevant source list by hand and reloading the CMake project (see `docs/developing.md` for per-IDE reload steps). Cross-directory shared TUs and all `tests/` sources are likewise listed explicitly.
- `assets/` holds runtime assets; `assets/CMakeLists.txt` packages them via `mod_pk3`.

## Docs

`docs/compiling.md`, `developing.md` (debugging + running against the engine), `testing.md`, `styleguide.md`. To playtest: `etl.x86_64 +set fs_basepath <build dir> +set fs_homepath <et install> +set fs_game etjump`.
