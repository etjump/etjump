# AGENTS.md

ETJump is a C/C++ (C++17) mod for Wolfenstein: Enemy Territory. CMake build, single repo, no monorepo. Built on Linux, Windows and macOS; produces the `cgame`, `qagame` and `ui` engine modules plus a packaged `.pk3`. Only mod game-code lives here — the game engine itself does not, so nothing in this repo can be run on its own.

Shell commands below are POSIX; on Windows the same CMake invocations work in `cmd`/PowerShell, just swap path separators and pass the config to multi-config generators (`--config Debug`, `ctest -C Debug`).

## Build & test

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # or Release; -G Ninja is optional
cmake --build build --parallel                     # --config Debug on multi-config generators
ctest --test-dir build --output-on-failure         # all tests (add -C Debug for multi-config)
./build/tests/tests --gtest_filter='TimerunSharedTests.*'  # single suite / test
```

- Module binaries land in `build/etjump/` next to a copy of `assets/`, named after the module with a platform suffix: `cgame.mp.x86_64.so` (Linux), `cgame_mp_x64.dll` (Windows), `cgame_mac` (macOS) — likewise for `qagame`/`ui`, and the packaged `etjump-<ver>.pk3`. The suffix comes from `cmake/PlatformDetails.cmake`; never hardcode a platform's library extension or arch in docs/scripts.
- Packaging targets: `cmake --build build --target mod_pk3` (pk3), `mod_release` (release zip). Release zips only include `qagame*` + pk3, not cgame/ui.
- `ctest` runs the single `tests` executable; CI runs it on every platform, passing `-C <config>` for multi-config generators. The binary is at `build/tests/tests` for single-config generators and `build/tests/<Config>/tests.exe` for multi-config ones.
- CMake options: `BUILD_TESTS` (ON by default), `USE_SANITIZERS` (enables ASAN+LSAN+UBSAN) and per-sanitizer `USE_ASAN`/`USE_LSAN`/`USE_UBSAN`, `USE_CLANG`, `USE_TEST_FAULTS` (developer-only fault injection, see `docs/developing.md`).
- Cross-compile toolchains live in `cmake/Toolchain-*.cmake` (x86 linux, mingw x86/x64 linux/windows).
- Do not add deps via system packages; vendored deps live in `deps/` and are wired up in the root `CMakeLists.txt`.

## Configure-time generated files (source tree gets modified!)

Running CMake **writes files into the source tree**, not just the build dir:
- `assets/ui/git_version.h` — from `git describe` (falls back to `VERSION.txt` when not a git repo / no tags).
- `assets/ui/changelog/version_headers.h` + one `assets/ui/changelog/<version>.txt` per H1 section — parsed from `changelog.md`.
- Version comes from `VERSION.txt` (`VERSION_MAJOR/MINOR/PATCH <n>` format, regex-parsed, keep the format) but `GAME_VERSION` string uses `git describe`, so releases need a git tag.

Implications: after adding assets, editing `changelog.md`, or changing `VERSION.txt`, re-run `cmake -B build` before building. These files are gitignored and regenerated on every configure, so there's nothing to commit or manually sync.

## changelog.md

Format is fragile — it's parsed at configure time by `cmake/ParseChangelog.cmake`, and malformed input breaks the build:
- One `#` H1 header per version; the parser splits sections on H1. `##` subheaders and bullet lists under them.
- Bullet-list entries only; nested lists indented with 2 spaces (not tabs).
- Entries end with the PR link, e.g.
  ``* `etj_feature` cvar to do a thing [#1234](https://github.com/etjump/etjump/pull/1234)``.
  Links are stripped from the in-game changelog, but keep them in the source.
- `**bold**` and `` `code` `` are highlighted in-game; `^` is the in-game color escape and the parser rewrites it, so avoid carets in prose.

Workflow: entries are backfilled in batches — once a handful of changes have accumulated since the last update (not tied to releases), the merged PRs get swept into `changelog.md` with their PR links. So a PR does not need to carry a changelog entry, and that is not a blocker either way; don't make a fuss about it in review. If an entry does land in a PR, the link can be filled in later — amend/force-push it once the number is known, or add it as a follow-up commit — rather than guessing the number upfront.

## Formatting & style

- Format with `clang-format -i` (repo `.clang-format`). **Only run it on code in `src/`** — never on `deps/` or `assets/` (shaders/menus). `git clang-format -i` is fine.
- Style guide (`docs/styleguide.md`): new code should be modern C++ — `ETJump::` namespaces (or more specific), camelCase functions/vars, PascalCase classes/enums, uppercase `constexpr` constants, no `using namespace std`, no `_`/`m_` member prefixes, no nested ternaries, early exits over deep nesting, `enum class` + explicit casts, `[[nodiscard]]` where applicable, fixed-width int types. Old C-style code is exempt; don't refactor unrelated files. The `.clang-tidy` relaxations are deliberate (legacy C), not to be re-enabled casually.
- Objects are constructed at module init and destroyed at shutdown, owned by a single per-module context struct (see `src/cgame/etj_cgame.h`); `shared_ptr` only for shared services/data, `unique_ptr` for sole ownership, raw refs for observers. `game` still needs migrating to this.
- New files start with the MIT header from `LICENSE` (see any `src/game/etj_*.cpp`); `scripts/include-license.sh` prepends it to any `etj_*.cpp`/`.h` that lacks a `Copyright` line. Leave existing headers alone — legacy SDK files carry their own.

## Tests

- GoogleTest. Add new tests in `tests/` and register both the test file and any new source it needs in `tests/CMakeLists.txt` (`add_executable(tests ...)` list) — the build links individual `src/` translation units, not whole modules.
- Test files include `src/game/etj_*.cpp` and `src/cgame/etj_*.cpp` sources directly; the game engine (q_shared, bg_*, etc.) is not linked, so tests are for self-contained modules only.
- Unit tests only cover self-contained logic. Don't try to verify game-specific behavior (physics, rendering, entity flow) via ctest — see playtesting below.

## Playtesting

There is no engine in this repo and no automated harness for gameplay behavior, so changes outside the unit-testable modules have to be run against a real engine (ET, ETe, ET: Legacy). The mod is loaded by the engine, not launched: `fs_homepath` points at an ET installation (engine + `etmain` with the game assets), `fs_basepath` at the build directory (the compiled modules), and `fs_game etjump` selects the mod. See `docs/developing.md` for the full setup, debugging and IDE specifics.

Two setups make this practical without an interactive session:

- **Dedicated server, for `src/game/` changes.** ET ships a headless dedicated server binary, so server-side work can be tested with no display at all — cvars/commands, entity spawning, physics, database/save handling, admin tooling. Point it at the same paths and drive it through the server console:
  ```sh
  # binary name varies by engine: etlded.x86_64 (ET: Legacy), ete-ded (ETe), etded (ET)
  etlded.x86_64 +set fs_basepath <build dir> +set fs_homepath <et install dir> +set fs_game etjump \
                +set logfile 2 +map <map>
  ```
  Server logs and the console are the observable interface; `logfile 2` also writes `etconsole.log` under `fs_homepath/<fs_game>/`.

- **Headless client, for client-side checks without a window.** The client runs with a real GL renderer but no visible window, so the mod loads exactly as it normally would:

  ```sh
  # ETe (no extra cvars needed)
  SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy \
    ./ete.x86_64 +set fs_basepath <build dir> +set fs_homepath <et install dir> +set fs_game etjump \
                 +set logfile 2 +devmap <map>

  # ET: Legacy: opengl1 only, and the mouse must be disabled
  SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy \
    ./etl.x86_64 +set cl_renderer opengl1 +set in_mouse 0 \
                 +set fs_basepath <build dir> +set fs_homepath <et install dir> +set fs_game etjump \
                 +set logfile 2 +devmap <map>
  ```

  - Platform support: ETL uses SDL on every platform, so this should work on Windows too as long as the SDL offscreen video driver is available. ETe only uses SDL on Linux — its Windows builds (including current release binaries) use the native backend, so there it is a Linux-only trick.
  - Engine specifics: on ETL the `vulkan` renderer does not come up headlessly and `opengl1` is required; ETe needs no renderer override. `SDL_VIDEODRIVER=dummy` does not work on either (`GLimp_Init() - could not load OpenGL subsystem`).
  - What it buys you: starting a local map loads qagame and cgame, so console output is a usable test channel for commands, cvar handling, config/script parsing and prints — `+set logfile 2` captures it in `etconsole.log` under `<fs_homepath>/<fs_game>/`.
  - What it can't do: anything visual — rendering, layout, frame timings — still needs a real session.

Neither is required for every change, but "it compiles" is not verification for gameplay code — state what you tested and how.

## Code layout

- `src/game/` — server (qagame): entities, physics, cvars/commands, database/save systems. `src/cgame/` — client: drawing, prediction, HUD. `src/ui/` — menu rendering. `cg_main.cpp`/`g_main.cpp`/`ui_main.cpp` are the `vmMain` entry points and `*_syscalls.cpp` wrap engine API calls.
- `etj_*.cpp/h` files are the newer, ETJump-specific modules; `bg_*`, `cg_*`, `g_*`, `q_*` are original SDK code that is still actively edited.
- Almost everything is free to edit, but the engine-facing contract is not: networked data (`usercmd_t`, `entityState_t`, `playerState_t`, the `netField_t` tables, configstrings, save-game layout) must stay compatible with the engines the mod supports, since the engine and mod are built and shipped separately. Extend it only where spare/custom fields allow (`bg_public.h`, `g_public.h`, `q_shared.h`); when unsure, check how the reference engines encode/decode it.
- CMake: module sources are listed explicitly in each module's `CMakeLists.txt` (`src/game`, `src/cgame`, `src/ui`) — adding a new `.cpp`/`.h` requires registering it in the relevant source list by hand and reloading the CMake project (see `docs/developing.md` for per-IDE reload steps). Cross-directory shared TUs and all `tests/` sources are likewise listed explicitly.
- `assets/` holds runtime assets; `assets/CMakeLists.txt` packages them via `mod_pk3`.

## Docs

`docs/compiling.md`, `developing.md` (running, debugging, IDEs, fault injection), `testing.md`, `styleguide.md`, `forking.md`. Keep them in sync when changing build options, targets or workflows. When behavior depends on the engine (syscalls, network protocol, rendering), the authority is the engine source (ET, ETe, ET: Legacy) — not this repo.
