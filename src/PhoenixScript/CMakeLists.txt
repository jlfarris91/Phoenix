include(FetchContent)

# ── wasm3 — lightweight WASM interpreter ─────────────────────────────────────
#
# wasm3 is not in vcpkg, so we fetch it from GitHub.
# The CMake target is "m3" (static library).
# Only the public header wasm3.h is needed by consumers of PhoenixScript —
# all wasm3 includes are confined to WasmWorldRuntime.cpp.

FetchContent_Declare(
    wasm3
    # URL download avoids GIT_REPOSITORY, which would cause CMake to invoke
    # find_package(Git) internally and create a Git::Git imported target with
    # no IMPORTED_LOCATION for custom configurations (e.g. ReleaseWithSymbols).
    URL                    https://github.com/wasm3/wasm3/archive/79d412ea5fcf92f0efe658d52827a0e0a96ff442.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

# Disable wasm3's own tests and WASI support before making it available.
# BUILD_WASI=none prevents wasm3 from fetching uvwasi/libuv (only needed for
# the standalone wasm3 binary, not for embedding).
set(BUILD_WASM3_TESTS   OFF CACHE BOOL   "" FORCE)
set(BUILD_WASM3_APPS    OFF CACHE BOOL   "" FORCE)
set(BUILD_WASI          "none" CACHE STRING "" FORCE)


FetchContent_MakeAvailable(wasm3)

# Enable wasm3 call-stack recording so m3_GetBacktrace() returns useful info.
# Must be set on the m3 target directly (not via add_compile_definitions, which
# only affects targets in the current directory).
target_compile_definitions(m3 PUBLIC d_m3RecordBacktraces=1)

# ── PhoenixScript ─────────────────────────────────────────────────────────────

file(GLOB_RECURSE PHOENIXSCRIPT_SRC "*.cpp" "*.h")

add_library(PhoenixScript ${PHOENIXSCRIPT_SRC})

target_include_directories(PhoenixScript
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(PhoenixScript
    PUBLIC
        PhoenixSim
    PRIVATE
        m3
)

# wasm3 exports its headers via target_include_directories on "m3",
# so consumers of PhoenixScript do NOT need to add wasm3 include paths.
