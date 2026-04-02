include(FetchContent)

# ── Lua 5.4 source ────────────────────────────────────────────────────────────
#
# Fetched from the official GitHub mirror.  FetchContent_Populate (not
# MakeAvailable) is used so CMake does not try to configure Lua's own
# CMakeLists.txt.

FetchContent_Declare(
    lua54
    # URL download avoids GIT_REPOSITORY, which would cause CMake to invoke
    # find_package(Git) internally and create a Git::Git imported target with
    # no IMPORTED_LOCATION for custom configurations (e.g. ReleaseWithSymbols).
    # The GitHub archive for v5.4.7 extracts with source files at the root,
    # matching the structure expected by LUA54_SOURCES below.
    URL                    https://github.com/lua/lua/archive/refs/tags/v5.4.7.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

# Suppress CMP0169 deprecation warning (CMake 3.30+): we intentionally use
# FetchContent_Populate to download without calling add_subdirectory, since
# Lua's CMakeLists.txt is not needed — we compile the source files directly.
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_GetProperties(lua54)
if(NOT lua54_POPULATED)
    FetchContent_Populate(lua54)
endif()

# Lua C sources — all except the standalone interpreter (lua.c) and
# standalone compiler (luac.c), which define their own main().
set(LUA54_SOURCES
    "${lua54_SOURCE_DIR}/lapi.c"
    "${lua54_SOURCE_DIR}/lauxlib.c"
    "${lua54_SOURCE_DIR}/lbaselib.c"
    "${lua54_SOURCE_DIR}/lcode.c"
    "${lua54_SOURCE_DIR}/lcorolib.c"
    "${lua54_SOURCE_DIR}/lctype.c"
    "${lua54_SOURCE_DIR}/ldebug.c"
    "${lua54_SOURCE_DIR}/ldo.c"
    "${lua54_SOURCE_DIR}/ldump.c"
    "${lua54_SOURCE_DIR}/lfunc.c"
    "${lua54_SOURCE_DIR}/lgc.c"
    "${lua54_SOURCE_DIR}/linit.c"
    "${lua54_SOURCE_DIR}/liolib.c"
    "${lua54_SOURCE_DIR}/llex.c"
    "${lua54_SOURCE_DIR}/lmathlib.c"
    "${lua54_SOURCE_DIR}/lmem.c"
    "${lua54_SOURCE_DIR}/loadlib.c"
    "${lua54_SOURCE_DIR}/lobject.c"
    "${lua54_SOURCE_DIR}/lopcodes.c"
    "${lua54_SOURCE_DIR}/loslib.c"
    "${lua54_SOURCE_DIR}/lparser.c"
    "${lua54_SOURCE_DIR}/lstate.c"
    "${lua54_SOURCE_DIR}/lstring.c"
    "${lua54_SOURCE_DIR}/lstrlib.c"
    "${lua54_SOURCE_DIR}/ltable.c"
    "${lua54_SOURCE_DIR}/ltablib.c"
    "${lua54_SOURCE_DIR}/ltm.c"
    "${lua54_SOURCE_DIR}/lundump.c"
    "${lua54_SOURCE_DIR}/lutf8lib.c"
    "${lua54_SOURCE_DIR}/lvm.c"
    "${lua54_SOURCE_DIR}/lzio.c"
)

# ── Emscripten check ──────────────────────────────────────────────────────────

# Search PATH first, then fall back to the standard emsdk install location
# ($EMSDK is set by emsdk_env / bootstrap.ps1 -Target emscripten).
# On Windows emcc ships as emcc.bat — list it explicitly since find_program
# only probes .exe/.com by default, not .bat.
#
# CMake caches find_program results — if emcc was not found on a previous
# configure, clear the cached NOTFOUND so that subsequent configures (after
# emsdk is installed) will re-search automatically.
if(NOT EMCC)
    unset(EMCC CACHE)
endif()
find_program(EMCC NAMES emcc emcc.bat
    HINTS "$ENV{EMSDK}/upstream/emscripten"
)
if(NOT EMCC)
    message(STATUS "PhoenixLuaRuntime: emcc not found — any call to add_lua_wasm_script() will be a fatal error")
    message(STATUS "  Run: build/bootstrap.ps1 -Target emscripten  (then re-run cmake)")
endif()

# ── add_lua_wasm_script ───────────────────────────────────────────────────────
#
# Compiles a .lua script + the Lua 5.4 interpreter into a standalone .wasm
# file loadable by FeatureScript / WasmEnvironment.
#
# Build pipeline:
#   1a. PhoenixWasmGen → host_api.h      (language-agnostic WASM imports)
#   1b. PhoenixLuaGen  → lua_bridge.c   (Lua stack marshaling)
#   2.  emcc(lua_wasm.c + lua_bridge.c + Lua srcs) → lua.wasm
#
# Usage:
#   add_lua_wasm_script(
#       OUTPUT  <path/to/output/LuaRunner.wasm>
#       [TARGET <cmake_target_that_depends_on_the_wasm>]
#   )
#
# Note: SCRIPT is accepted for backward compatibility but ignored.
# Scripts are loaded at runtime by the host via GetScriptBuffer/LoadScript exports.

function(add_lua_wasm_script)
    cmake_parse_arguments(ARG "" "SCRIPT;OUTPUT;TARGET" "" ${ARGN})

    if(NOT ARG_OUTPUT)
        message(FATAL_ERROR "add_lua_wasm_script: OUTPUT argument is required")
    endif()

    if(NOT EMCC)
        message(FATAL_ERROR
            "add_lua_wasm_script: emcc not found — cannot build LuaRunner.wasm.\n"
            "  Run: build/bootstrap.ps1 -Target emscripten  (then re-run cmake)")
    endif()

    # lua54_SOURCE_DIR was set in tools/PhoenixLuaRuntime's directory scope by
    # FetchContent_Populate.  Functions execute in the *calling* scope, so the
    # variable is not visible here.  FetchContent_GetProperties re-resolves it
    # from the global FetchContent state into the current (function) scope.
    FetchContent_GetProperties(lua54)
    if(NOT lua54_POPULATED)
        message(FATAL_ERROR "add_lua_wasm_script: lua54 is not populated — "
                            "include tools/PhoenixLuaRuntime before calling this function.")
    endif()

    set(_lua_src "${lua54_SOURCE_DIR}")
    set(_lua54_sources
        "${_lua_src}/lapi.c"     "${_lua_src}/lauxlib.c"  "${_lua_src}/lbaselib.c"
        "${_lua_src}/lcode.c"    "${_lua_src}/lcorolib.c" "${_lua_src}/lctype.c"
        "${_lua_src}/ldebug.c"   "${_lua_src}/ldo.c"      "${_lua_src}/ldump.c"
        "${_lua_src}/lfunc.c"    "${_lua_src}/lgc.c"      "${_lua_src}/linit.c"
        "${_lua_src}/liolib.c"   "${_lua_src}/llex.c"     "${_lua_src}/lmathlib.c"
        "${_lua_src}/lmem.c"     "${_lua_src}/loadlib.c"  "${_lua_src}/lobject.c"
        "${_lua_src}/lopcodes.c" "${_lua_src}/loslib.c"   "${_lua_src}/lparser.c"
        "${_lua_src}/lstate.c"   "${_lua_src}/lstring.c"  "${_lua_src}/lstrlib.c"
        "${_lua_src}/ltable.c"   "${_lua_src}/ltablib.c"  "${_lua_src}/ltm.c"
        "${_lua_src}/lundump.c"  "${_lua_src}/lutf8lib.c" "${_lua_src}/lvm.c"
        "${_lua_src}/lzio.c"
    )

    # ── Step 1a: Generate host_api.h (PhoenixWasmGen) ──────────────────────
    set(HOST_API_H   "${CMAKE_CURRENT_BINARY_DIR}/host_api.h")
    set(LUA_BRIDGE_C "${CMAKE_CURRENT_BINARY_DIR}/lua_bridge.c")

    add_custom_command(
        OUTPUT  "${HOST_API_H}"
        COMMAND $<TARGET_FILE:PhoenixWasmGen>
            --host-api "${HOST_API_H}"
        DEPENDS PhoenixWasmGen GenerateAPI
        COMMENT "Generating host_api.h from TypeRegistry"
    )

    # ── Step 1b: Generate lua_bridge.c + Phoenix.d.lua (PhoenixLuaGen) ─────────
    set(LUA_DEFS_FILE "${CMAKE_CURRENT_BINARY_DIR}/Phoenix.d.lua")
    add_custom_command(
        OUTPUT  "${LUA_BRIDGE_C}" "${LUA_DEFS_FILE}"
        COMMAND $<TARGET_FILE:PhoenixLuaGen>
            --lua-bridge "${LUA_BRIDGE_C}"
            --lua-defs   "${LUA_DEFS_FILE}"
        DEPENDS PhoenixLuaGen GenerateAPI
        COMMENT "Generating lua_bridge.c from TypeRegistry"
    )

    # ── Step 2: Compile → .wasm ─────────────────────────────────────────────
    #
    # The script is no longer embedded at compile time.  lua_wasm.c exports
    # GetScriptBuffer()/LoadScript(len) so the host can push Lua source bytes
    # into WASM memory at runtime, enabling file-based loading and hot-reload.
    set(WRAPPER_C "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lua_wasm.c")

    add_custom_command(
        OUTPUT  "${ARG_OUTPUT}"
        COMMAND ${EMCC}
            # Source files — lua_wasm.c (framework) + lua_bridge.c (generated)
            "${WRAPPER_C}"
            "${LUA_BRIDGE_C}"
            ${_lua54_sources}
            # Include paths — build dir first so generated headers take precedence
            -I "${CMAKE_CURRENT_BINARY_DIR}"
            -I "${_lua_src}"
            -I "${CMAKE_CURRENT_FUNCTION_LIST_DIR}"
            # Lua configuration
            -DLUA_32BITS                        # 32-bit ints/floats for determinism
            # Emscripten WASM flags
            --no-entry                          # no main()
            -sSTANDALONE_WASM=1                # pure .wasm, no JS glue
            "-sINITIAL_MEMORY=16777216"         # 16 MiB (fixed) — Lua 5.4 + libs need ~8–12 MiB
            "-sMAXIMUM_MEMORY=16777216"         # must equal INITIAL for snapshot/restore
            "-sALLOW_MEMORY_GROWTH=0"           # fixed size required for deterministic memcpy
            "-sERROR_ON_UNDEFINED_SYMBOLS=0"    # host imports resolved at runtime by wasm3
            # no -sSUPPORT_LONGJMP override — default emscripten mode emits invoke_*
            # wrappers that WasmEnvironment.cpp dispatches via DispatchTableCall()
            -O2 -g
            -o "${ARG_OUTPUT}"
        DEPENDS
            "${HOST_API_H}"
            "${LUA_BRIDGE_C}"
            "${WRAPPER_C}"
            ${_lua54_sources}
        COMMENT "Compiling Lua-WASM interpreter → ${ARG_OUTPUT}"
    )

    # ── Step 4: Named target ────────────────────────────────────────────────
    if(ARG_TARGET)
        set(WASM_TARGET_NAME "${ARG_TARGET}_lua_wasm")
    else()
        get_filename_component(SCRIPT_STEM "${ARG_SCRIPT}" NAME_WE)
        set(WASM_TARGET_NAME "lua_wasm_${SCRIPT_STEM}")
    endif()

    add_custom_target("${WASM_TARGET_NAME}" ALL DEPENDS "${ARG_OUTPUT}")

    if(ARG_TARGET AND TARGET "${ARG_TARGET}")
        add_dependencies("${ARG_TARGET}" "${WASM_TARGET_NAME}")
    endif()
endfunction()
