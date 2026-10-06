#!/usr/bin/env python3
"""Fail if warnings are disabled for any first-party target.

The bundled dependencies are deliberately built with warnings off - see the
comment above cxx_compiler_opts_w0 in the root CMakeLists.txt. That
suppression must never reach cgame, qagame, ui or tests, which is what
happens if cxx_compiler_opts_w0 is linked into a header-only INTERFACE
library: the option lands in the consumer's link interface and silently
disables every warning for the whole module.

Reads a compile_commands.json, so it only needs a CMake configure, not a
build. Requires CMAKE_EXPORT_COMPILE_COMMANDS (Ninja and Makefile
generators; not available for the Visual Studio generator).

Usage: check-warnings-enabled.py [path/to/compile_commands.json]
"""

import json
import re
import sys

FIRST_PARTY = re.compile(r"/(cgame|qagame|ui|tests)\.dir/")
# a bare -w (GCC/Clang) or /W0 (MSVC); -Wfoo and -Werror are not affected
DISABLED = re.compile(r"(?:^|\s)-w(?:\s|$)|/W0(?:\s|$)")


def relative(path):
    """Trim the path down to something readable, e.g. src/cgame/cg_main.cpp."""
    marker = path.rfind("/src/")
    return path[marker + 1 :] if marker != -1 else path


def main(path):
    try:
        with open(path, encoding="utf-8") as f:
            commands = json.load(f)
    except (OSError, json.JSONDecodeError) as error:
        # e.g. the export is missing or the path is wrong - never report ok
        sys.exit(f"error: cannot read {path}: {error}")

    first_party = []
    for entry in commands:
        match = FIRST_PARTY.search(entry["command"])
        if match is not None:
            first_party.append((match.group(1), entry["file"], entry["command"]))
    if not first_party:
        # a guard that cannot see anything must not report success
        sys.exit(f"error: no first-party compile commands found in {path}")

    disabled = [
        (target, source)
        for target, source, command in first_party
        if DISABLED.search(command)
    ]

    if disabled:
        by_target = {}
        for target, source in disabled:
            by_target.setdefault(target, []).append(source)

        print("error: warnings are disabled for first-party targets:", file=sys.stderr)
        for target in sorted(by_target):
            sources = by_target[target]
            example = f" (e.g. {relative(sources[0])})" if sources else ""
            print(
                f"  {target}: {len(sources)} translation units{example}",
                file=sys.stderr,
            )
        print(
            "\ncxx_compiler_opts_w0 (WARN 0 -> -w, /W0 on MSVC) is reaching these\n"
            "targets. Bundled dependencies may only use it with PRIVATE; a\n"
            "header-only dependency must suppress its own headers with SYSTEM\n"
            "includes instead - see the comment in the root CMakeLists.txt.",
            file=sys.stderr,
        )
        sys.exit(1)

    print(f"ok: warnings enabled for {len(first_party)} first-party translation units")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "build/compile_commands.json")
