"""
fix_includes2.py - Phase A + Phase B include path migrations.

Phase A: Phoenix.Sim/Reflection/ -> Phoenix/Reflection/
Phase B: Phoenix.Sim/<subdir>/ -> Phoenix.Sim.<Project>/<subdir-in-new-project>/
         (files move from src/Phoenix.Sim/ECS/ to src/Phoenix.Sim.ECS/, so the
          include prefix changes from "Phoenix.Sim/ECS/" to "Phoenix.Sim.ECS/")

Run from repo root.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(ROOT)  # go up from tools/ to repo root

EXTENSIONS = {".h", ".cpp", ".inl"}

SEARCH_DIRS = [
    "src",
    "apps",
    "tests",
    "tools",
]

# Order matters — most specific first.
# Each entry: (old_include_prefix, new_include_prefix)
REPLACEMENTS = [
    # Phase A: Reflection moves from Phoenix.Sim to Phoenix
    ("Phoenix.Sim/Reflection/", "Phoenix/Reflection/"),

    # Phase B: Extracted feature subdirectories
    # Navigation -> Nav (directory renamed too)
    ("Phoenix.Sim/Navigation/", "Phoenix.Sim.Nav/"),
    # The rest keep their subdir name but move to a sibling project directory
    ("Phoenix.Sim/Blackboard/", "Phoenix.Sim.Blackboard/"),
    ("Phoenix.Sim/Strings/",    "Phoenix.Sim.Strings/"),
    ("Phoenix.Sim/Debug/",      "Phoenix.Sim.Debug/"),
    ("Phoenix.Sim/Mesh/",       "Phoenix.Sim.Mesh/"),
    ("Phoenix.Sim/ECS/",        "Phoenix.Sim.ECS/"),
    ("Phoenix.Sim/LDS/",        "Phoenix.Sim.LDS/"),
]

def fix_file(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        original = f.read()

    content = original
    for old, new in REPLACEMENTS:
        # Match both "..." and <...> include styles
        pattern = r'(#\s*include\s*[<"])' + re.escape(old)
        replacement = r'\g<1>' + new
        content = re.sub(pattern, replacement, content)

    if content != original:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(content)
        return True
    return False

total_files = 0
changed_files = 0
for search_dir in SEARCH_DIRS:
    full_dir = os.path.join(ROOT, search_dir)
    if not os.path.isdir(full_dir):
        continue
    for dirpath, _, filenames in os.walk(full_dir):
        for fname in filenames:
            ext = os.path.splitext(fname)[1].lower()
            if ext in EXTENSIONS:
                total_files += 1
                if fix_file(os.path.join(dirpath, fname)):
                    changed_files += 1

print(f"Scanned {total_files} files, updated {changed_files}.")
