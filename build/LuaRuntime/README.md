# build/LuaRuntime

CMake build helper for compiling Lua 5.4 into a WASM binary (`lua.wasm`).

## What it is

This is **not** a C++ library or native executable. It is a CMake module that:

1. Fetches Lua 5.4 source via `FetchContent`
2. Defines the `add_lua_wasm_script()` CMake function
3. Provides `lua_wasm.c` — the C wrapper compiled by emscripten into `lua.wasm`

The output `lua.wasm` is the Lua interpreter that runs inside the simulation's
WASM sandbox (`Phoenix.Sim.Script` / `Phoenix.Sim.Lua`).

## Build pipeline

```
Phoenix.Build.WasmGen  →  host_api.h      (WASM import declarations)
Phoenix.Build.LuaGen   →  lua_bridge.c    (Lua ↔ host marshaling)
emcc(lua_wasm.c + lua_bridge.c + Lua 5.4) →  lua.wasm
```

## Usage

In a `CMakeLists.txt` that needs a Lua WASM binary:

```cmake
# build/LuaRuntime must be add_subdirectory'd before calling this.
add_lua_wasm_script(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/lua.wasm"
    TARGET  MyTarget
    DEFS_DEST "${CMAKE_SOURCE_DIR}/src/Phoenix.Sim.Lua/Phoenix.d.lua"
)
```

## Requirements

Emscripten (`emcc`) must be on PATH. Run `build/bootstrap.ps1 -Target emscripten`
if it is not installed.
