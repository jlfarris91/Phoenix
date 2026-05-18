include(FetchContent)
# CMP0169: FetchContent_Populate(name) with declared details is deprecated in
# CMake 3.30+. Set OLD to suppress the warning while we bypass wasm3's
# top-level CMakeLists.txt (see comment below).
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

# ── wasm3 — lightweight WASM interpreter ─────────────────────────────────────
#
# wasm3 is not in vcpkg, so we fetch it from GitHub.
# The CMake target is "m3" (static library).
# Only the public header wasm3.h is needed by consumers of Phoenix.Sim.Script —
# all wasm3 includes are confined to WasmWorldRuntime.cpp.

FetchContent_Declare(
    wasm3
    # URL download avoids GIT_REPOSITORY, which would cause CMake to invoke
    # find_package(Git) internally and create a Git::Git imported target with
    # no IMPORTED_LOCATION for custom configurations (e.g. ReleaseWithSymbols).
    URL                    https://github.com/wasm3/wasm3/archive/79d412ea5fcf92f0efe658d52827a0e0a96ff442.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

# Bypass wasm3's top-level CMakeLists.txt: it calls project(wasm3) which
# re-runs compiler detection and fails on Windows when emcc is not in PATH
# as a native executable. The source/CMakeLists.txt is a minimal add_library
# with no project() call, so we use that directly.
FetchContent_GetProperties(wasm3)
if(NOT wasm3_POPULATED)
    FetchContent_Populate(wasm3)
    add_subdirectory(${wasm3_SOURCE_DIR}/source ${wasm3_BINARY_DIR}/source)
endif()

# Enable wasm3 call-stack recording so m3_GetBacktrace() returns useful info.
# Restricted to Release builds — in Debug, backtrace recording calls
# PushBacktraceFrame at every WASM op, which roughly doubles native stack
# usage and causes a stack overflow when running Lua (a large WASM binary).
target_compile_definitions(m3 PUBLIC
    $<$<NOT:$<CONFIG:Debug>>:d_m3RecordBacktraces=1>
)

# Disable wasm3 debug assertions (NASSERTS).
# wasm3's PushBacktraceFrame assert fires when a trap propagates through
# our DispatchTableCall direct-dispatch path (invoke_* trampolines).
# The assert is a false-positive: the backtrace mechanism gets confused by
# compiled WASM invoked directly from a host function rather than via m3_CallV.
target_compile_definitions(m3 PRIVATE NASSERTS)

# Always optimize wasm3, even in Debug builds.
# wasm3's threaded dispatch chains C function pointers for every WASM op.
# Without optimization, MSVC generates large debug frames for each op, and
# running a large WASM binary like the Lua VM exhausts the stack even at 32 MB.
# Optimization collapses the per-instruction frames and reduces stack depth by
# ~10x, making Debug builds viable.

# ── Phoenix.Sim.Script ─────────────────────────────────────────────────────────────

file(GLOB_RECURSE PHOENIXSCRIPT_SRC "*.cpp" "*.h")

add_library(Phoenix.Sim.Script ${PHOENIXSCRIPT_SRC})

target_include_directories(Phoenix.Sim.Script
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
)

target_link_libraries(Phoenix.Sim.Script
    PUBLIC
        Phoenix.Sim
    PRIVATE
        m3
)

# wasm3 exports its headers via target_include_directories on "m3",
# so consumers of Phoenix.Sim.Script do NOT need to add wasm3 include paths.
