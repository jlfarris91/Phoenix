#!/usr/bin/env python3
"""Fix legacy PhoenixXxx/ include prefixes to dot-notation Phoenix.Xxx/ equivalents."""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent

REPLACEMENTS = {
    "PhoenixSim/":      "Phoenix.Sim/",
    "PhoenixPhysics/":  "Phoenix.Sim.Physics/",
    "PhoenixRTS/":      "Phoenix.Sim.RTS/",
    "PhoenixSteering/": "Phoenix.Sim.Steering/",
    "PhoenixScript/":   "Phoenix.Sim.Script/",
    "PhoenixLua/":      "Phoenix.Sim.Lua/",
}

PATTERN = re.compile(
    r'(#include\s+[<"])'
    r'(' + '|'.join(re.escape(k) for k in REPLACEMENTS) + r')'
)

SEARCH_DIRS = ["src", "tools", "apps", "tests"]

def fix_file(path: Path, dry_run: bool) -> int:
    text = path.read_text(encoding="utf-8", errors="replace")
    new_text = PATTERN.sub(
        lambda m: m.group(1) + REPLACEMENTS[m.group(2)],
        text
    )
    if new_text == text:
        return 0
    count = len(PATTERN.findall(text))
    if not dry_run:
        path.write_text(new_text, encoding="utf-8")
    print(f"  {'[dry]' if dry_run else '[fix]'} {path.relative_to(ROOT)}  ({count} replacement{'s' if count != 1 else ''})")
    return count

def main():
    dry_run = "--dry-run" in sys.argv or "-n" in sys.argv
    total_files = 0
    total_replacements = 0

    for search_dir in SEARCH_DIRS:
        for ext in ("*.h", "*.cpp", "*.inl"):
            for path in (ROOT / search_dir).rglob(ext):
                n = fix_file(path, dry_run)
                if n:
                    total_files += 1
                    total_replacements += n

    print(f"\n{'Dry run — ' if dry_run else ''}Fixed {total_replacements} include(s) across {total_files} file(s).")
    if dry_run:
        print("Run without --dry-run to apply changes.")

if __name__ == "__main__":
    main()
