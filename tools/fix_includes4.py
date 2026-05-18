"""fix_includes4.py - Redirect Phoenix.Sim headers that moved to Phoenix/"""

import os, re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXTENSIONS = {".h", ".cpp", ".inl"}
SEARCH_DIRS = ["src", "apps", "tests", "tools"]

REPLACEMENTS = [
    ("Phoenix.Sim/FixedPoint/",            "Phoenix/FixedPoint/"),
    ("Phoenix.Sim/Containers/MPMCQueue.h", "Phoenix/Containers/MPMCQueue.h"),
    ("Phoenix.Sim/Containers/Optional.h",  "Phoenix/Containers/Optional.h"),
    ("Phoenix.Sim/CTZ.h",                  "Phoenix/CTZ.h"),
    ("Phoenix.Sim/Corners.h",              "Phoenix/Corners.h"),
    ("Phoenix.Sim/FPSCalc.h",              "Phoenix/FPSCalc.h"),
    ("Phoenix.Sim/OffsetRef.h",            "Phoenix/OffsetRef.h"),
    ("Phoenix.Sim/Delegates.h",            "Phoenix/Delegates.h"),
    ("Phoenix.Sim/Logging.h",              "Phoenix/Logging.h"),
    ("Phoenix.Sim/Profiling.h",            "Phoenix/Profiling.h"),
    ("Phoenix.Sim/Parallel.h",             "Phoenix/Parallel.h"),
    ("Phoenix.Sim/Color.h",                "Phoenix/Color.h"),
    ("Phoenix.Sim/MortonCode.h",           "Phoenix/MortonCode.h"),
    ("Phoenix.Sim/Random.h",               "Phoenix/Random.h"),
]

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
