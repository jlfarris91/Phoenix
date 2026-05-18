#!/usr/bin/env python3
"""Rename CMake target names from old PhoenixXxx convention to Phoenix.Xxx dot-notation."""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent

# Order matters: longer/more-specific names must come before shorter prefixes
# so "PhoenixEditor.Core" is replaced before "PhoenixEditor".
REPLACEMENTS = [
    ("PhoenixRuntime.SDL3",  "Phoenix.Runtime.SDL3"),
    ("PhoenixRuntime.Core",  "Phoenix.Runtime"),
    ("PhoenixEditor.ImGui",  "Phoenix.Editor.ImGui"),
    ("PhoenixEditor.Core",   "Phoenix.Editor"),
    ("PhoenixApp.Core",      "Phoenix.App"),
    ("PhoenixSteering",      "Phoenix.Sim.Steering"),
    ("PhoenixPhysics",       "Phoenix.Sim.Physics"),
    ("PhoenixScript",        "Phoenix.Sim.Script"),
    ("PhoenixLua",           "Phoenix.Sim.Lua"),
    ("PhoenixRTS",           "Phoenix.Sim.RTS"),
    ("PhoenixSim",           "Phoenix.Sim"),
    ("PhoenixAPIGen",        "Phoenix.Tools.APIGen"),
    ("PhoenixLuaGen",        "Phoenix.Tools.LuaGen"),
    ("PhoenixWasmGen",       "Phoenix.Tools.WasmGen"),
    ("PhoenixTests",         "Phoenix.Tests"),
    # Exe last — most general, must not match the library names above
    ("PhoenixEditor",        "Phoenix.Editor.App"),
]

# Match target names as whole words (not preceded/followed by identifier chars or dots)
def make_pattern(old):
    return re.compile(r'(?<![.\w])' + re.escape(old) + r'(?![.\w])')

PATTERNS = [(make_pattern(old), new) for old, new in REPLACEMENTS]

SEARCH_DIRS = ["src", "apps", "tests", "tools"]

def fix_file(path: Path, dry_run: bool) -> int:
    text = path.read_text(encoding="utf-8", errors="replace")
    new_text = text
    count = 0
    for pattern, new in PATTERNS:
        replaced = pattern.sub(new, new_text)
        count += len(pattern.findall(new_text))
        new_text = replaced
    if new_text == text:
        return 0
    if not dry_run:
        path.write_text(new_text, encoding="utf-8")
    print(f"  {'[dry]' if dry_run else '[fix]'} {path.relative_to(ROOT)}  ({count} replacement{'s' if count != 1 else ''})")
    return count

def main():
    dry_run = "--dry-run" in sys.argv or "-n" in sys.argv
    total_files = 0
    total_replacements = 0

    for search_dir in SEARCH_DIRS:
        for path in (ROOT / search_dir).rglob("CMakeLists.txt"):
            n = fix_file(path, dry_run)
            if n:
                total_files += 1
                total_replacements += n

    # Also fix the root CMakeLists.txt
    n = fix_file(ROOT / "CMakeLists.txt", dry_run)
    if n:
        total_files += 1
        total_replacements += n

    print(f"\n{'Dry run — ' if dry_run else ''}Fixed {total_replacements} name(s) across {total_files} file(s).")

if __name__ == "__main__":
    main()
