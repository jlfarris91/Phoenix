# ── PhoenixWasmGen ────────────────────────────────────────────────────────────
#
# Native executable that walks the live TypeRegistry and emits:
#   host_api.h   — language-agnostic WASM import declarations
#
# Used as a CMake build step by tools/PhoenixLuaRuntime.
#
# Linking model:
#   /WHOLEARCHIVE forces every .obj from each static library to be included so
#   every PHX_DEFINE_TYPE static initializer runs and populates the TypeRegistry.

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
    pantor::inja
)

if(MSVC)
    target_link_options(PhoenixWasmGen PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixRTS>
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixPhysics>
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSteering>
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSim>
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixScript>
    )
elseif(APPLE)
    target_link_options(PhoenixWasmGen PRIVATE
        -Wl,-force_load,$<TARGET_FILE:PhoenixRTS>
        -Wl,-force_load,$<TARGET_FILE:PhoenixPhysics>
        -Wl,-force_load,$<TARGET_FILE:PhoenixSteering>
        -Wl,-force_load,$<TARGET_FILE:PhoenixSim>
        -Wl,-force_load,$<TARGET_FILE:PhoenixScript>
    )
else()
    target_link_options(PhoenixWasmGen PRIVATE
        -Wl,--whole-archive
        $<TARGET_FILE:PhoenixRTS>
        $<TARGET_FILE:PhoenixPhysics>
        $<TARGET_FILE:PhoenixSteering>
        $<TARGET_FILE:PhoenixSim>
        $<TARGET_FILE:PhoenixScript>
        -Wl,--no-whole-archive
    )
endif()

set_property(TARGET PhoenixWasmGen PROPERTY FOLDER "Tools")
