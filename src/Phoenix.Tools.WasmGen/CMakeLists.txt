# ── Phoenix.Tools.WasmGen ────────────────────────────────────────────────────────────
#
# Native executable that walks the live TypeRegistry and emits:
#   host_api.h   — language-agnostic WASM import declarations
#
# Used as a CMake build step by tools/PhoenixLuaRuntime.
#
# Linking model:
#   /WHOLEARCHIVE forces every .obj from each static library to be included so
#   every PHX_DEFINE_TYPE static initializer runs and populates the TypeRegistry.

add_executable(Phoenix.Tools.WasmGen
    "${CMAKE_CURRENT_LIST_DIR}/PhoenixWasmGen.cpp"
)

target_include_directories(Phoenix.Tools.WasmGen PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

# Link all libraries and force every object file to be included so that every
# PHX_DEFINE_TYPE static initializer runs and populates the TypeRegistry.
# Without this, the linker dead-strips registration TUs that are not referenced
# by any symbol in PhoenixWasmGen.cpp, causing types to be absent from the output.
target_link_libraries(Phoenix.Tools.WasmGen PRIVATE
    Phoenix.Sim.RTS
    Phoenix.Sim.Physics
    Phoenix.Sim.Steering
    Phoenix.Sim
    Phoenix.Sim.Script
    pantor::inja
)

if(MSVC)
    target_link_options(Phoenix.Tools.WasmGen PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.RTS>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.Physics>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.Steering>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.Script>
    )
elseif(APPLE)
    target_link_options(Phoenix.Tools.WasmGen PRIVATE
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.RTS>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.Physics>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.Steering>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.Script>
    )
else()
    target_link_options(Phoenix.Tools.WasmGen PRIVATE
        -Wl,--whole-archive
        $<TARGET_FILE:Phoenix.Sim.RTS>
        $<TARGET_FILE:Phoenix.Sim.Physics>
        $<TARGET_FILE:Phoenix.Sim.Steering>
        $<TARGET_FILE:Phoenix.Sim>
        $<TARGET_FILE:Phoenix.Sim.Script>
        -Wl,--no-whole-archive
    )
endif()

set_property(TARGET Phoenix.Tools.WasmGen PROPERTY FOLDER "Tools")
