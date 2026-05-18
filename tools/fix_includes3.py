"""fix_includes3.py - Redirect Phoenix.Sim root-level headers to Phoenix/"""

import os, re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXTENSIONS = {".h", ".cpp", ".inl"}
SEARCH_DIRS = ["src", "apps", "tests", "tools"]

# These files moved from Phoenix.Sim/ root to Phoenix/
MOVED = ["Platform.h", "Flags.h", "Hashing.h", "Name.h", "Utils.h"]

REPLACEMENTS = [(f"Phoenix.Sim/{f}", f"Phoenix/{f}") for f in MOVED]

def fix_file(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        original = f.read()
    content = original
    for old, new in REPLACEMENTS:
        pattern = r'(#\s*include\s*[<"])' + re.escape(old)
        content = re.sub(pattern, r'\g<1>' + new, content)
    if content != original:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(content)
        return True
    return False

total = changed = 0
for d in SEARCH_DIRS:
    full = os.path.join(ROOT, d)
    if not os.path.isdir(full):
        continue
    for dp, _, files in os.walk(full):
        for fn in files:
            if os.path.splitext(fn)[1].lower() in EXTENSIONS:
                total += 1
                if fix_file(os.path.join(dp, fn)):
                    changed += 1

print(f"Scanned {total} files, updated {changed}.")
