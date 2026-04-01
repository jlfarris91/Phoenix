# ── PhoenixWasmGen ────────────────────────────────────────────────────────────
#
# Native executable that walks the live TypeRegistry and emits:
#   host_api.h   — language-agnostic WASM import declarations
#   lua_bridge.c — Lua stack-marshaling wrappers + register_phoenix_api()
#
# Used as a CMake build step by src/PhoenixLuaWasm (and any future
# language-specific WASM toolchain that needs the host API).
#
# Linking model:
#   GenerateWasmApi.cpp calls for each registered type, which forces the linker to include those TUs.
#   The static initializers in those TUs then populate the TypeRegistry with all member registrations.

add_executable(PhoenixWasmGen
    "${CMAKE_CURRENT_LIST_DIR}/PhoenixWasmGen.cpp"
)

target_include_directories(PhoenixWasmGen PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

# Link all libraries and force every object file to be included so that every
# PHX_DEFINE_TYPE static initializer runs and populates the TypeRegistry.
# Without this, the linker dead-strips registration TUs that are not referenced
# by any symbol in PhoenixWasmGen.cpp, causing types to be absent from the output.
target_link_libraries(PhoenixWasmGen PRIVATE
    PhoenixRTS
    PhoenixPhysics
    PhoenixSteering
    PhoenixSim
    PhoenixScript
)

# MSVC: /WHOLEARCHIVE:<lib> forces all .obj files from each static library to
# be included, regardless of whether their symbols are directly referenced.
target_link_options(PhoenixWasmGen PRIVATE
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixRTS>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixPhysics>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSteering>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSim>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixScript>
)

set_property(TARGET PhoenixWasmGen PROPERTY FOLDER "Tools")
