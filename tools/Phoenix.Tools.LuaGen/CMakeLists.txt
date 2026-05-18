# ── Phoenix.Tools.LuaGen ─────────────────────────────────────────────────────────────
#
# Native executable that walks the live TypeRegistry and emits:
#   lua_bridge.c — Lua stack-marshaling wrappers + register_phoenix_api()
#
# Used as a CMake build step by tools/PhoenixLuaRuntime.
#
# Linking model:
#   /WHOLEARCHIVE forces every .obj from each static library to be included so
#   every PHX_DEFINE_TYPE static initializer runs and populates the TypeRegistry.

add_executable(Phoenix.Tools.LuaGen
    "${CMAKE_CURRENT_LIST_DIR}/PhoenixLuaGen.cpp"
)

target_include_directories(Phoenix.Tools.LuaGen PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Tools.LuaGen PRIVATE
    Phoenix.Sim.RTS
    Phoenix.Sim.Physics
    Phoenix.Sim.Steering
    Phoenix.Sim
    Phoenix.Sim.Script
    pantor::inja
)

if(MSVC)
    target_link_options(Phoenix.Tools.LuaGen PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.RTS>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.Physics>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.Steering>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.Script>
    )
elseif(APPLE)
    target_link_options(Phoenix.Tools.LuaGen PRIVATE
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.RTS>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.Physics>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.Steering>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.Script>
    )
else()
    target_link_options(Phoenix.Tools.LuaGen PRIVATE
        -Wl,--whole-archive
        $<TARGET_FILE:Phoenix.Sim.RTS>
        $<TARGET_FILE:Phoenix.Sim.Physics>
        $<TARGET_FILE:Phoenix.Sim.Steering>
        $<TARGET_FILE:Phoenix.Sim>
        $<TARGET_FILE:Phoenix.Sim.Script>
        -Wl,--no-whole-archive
    )
endif()

set_property(TARGET Phoenix.Tools.LuaGen PROPERTY FOLDER "Tools")
