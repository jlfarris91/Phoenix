# ── PhoenixLuaGen ─────────────────────────────────────────────────────────────
#
# Native executable that walks the live TypeRegistry and emits:
#   lua_bridge.c — Lua stack-marshaling wrappers + register_phoenix_api()
#
# Used as a CMake build step by tools/PhoenixLuaRuntime.
#
# Linking model:
#   /WHOLEARCHIVE forces every .obj from each static library to be included so
#   every PHX_DEFINE_TYPE static initializer runs and populates the TypeRegistry.

add_executable(PhoenixLuaGen
    "${CMAKE_CURRENT_LIST_DIR}/PhoenixLuaGen.cpp"
)

target_include_directories(PhoenixLuaGen PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(PhoenixLuaGen PRIVATE
    PhoenixRTS
    PhoenixPhysics
    PhoenixSteering
    PhoenixSim
    PhoenixScript
    pantor::inja
)

target_link_options(PhoenixLuaGen PRIVATE
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixRTS>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixPhysics>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSteering>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSim>
    /WHOLEARCHIVE:$<TARGET_FILE:PhoenixScript>
)

set_property(TARGET PhoenixLuaGen PROPERTY FOLDER "Tools")
