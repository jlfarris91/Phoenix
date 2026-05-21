file(GLOB_RECURSE PHOENIXRTS2_SOURCES "src/*.cpp" "src/*.h")

phx_add_game(PhoenixRTS2
    SOURCES ${PHOENIXRTS2_SOURCES}
)

find_package(Tracy CONFIG REQUIRED)

target_compile_definitions(PhoenixRTS2 PRIVATE
    TRACY_ENABLE
    PHX_PROFILE_ENABLE
    IMGUI_DEFINE_MATH_OPERATORS
)

target_compile_options(PhoenixRTS2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

target_link_libraries(PhoenixRTS2 PRIVATE
    Phoenix.Sim
    Phoenix.Sim.Blackboard
    Phoenix.Sim.Debug
    Phoenix.Sim.Tasks
    Phoenix.Sim.ECS
    Phoenix.Sim.LDS
    Phoenix.Sim.Nav
    Phoenix.Sim.Strings
    Phoenix.Sim.Physics
    Phoenix.Sim.Steering
    Phoenix.Sim.Lua
    Phoenix.Sim.RTS
    Phoenix.Sim.Script
    Tracy::TracyClient
)

if(MSVC)
    target_link_options(PhoenixRTS2 PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.RTS>
        /STACK:134217728
    )
elseif(APPLE)
    target_link_options(PhoenixRTS2 PRIVATE
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.RTS>
    )
else()
    target_link_options(PhoenixRTS2 PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:Phoenix.Sim> $<TARGET_FILE:Phoenix.Sim.RTS> -Wl,--no-whole-archive
    )
endif()

set(LUA_WASM "${CMAKE_CURRENT_BINARY_DIR}/lua.wasm")
add_lua_wasm_script(
    OUTPUT    "${LUA_WASM}"
    TARGET    PhoenixRTS2
    DEFS_DEST "${CMAKE_SOURCE_DIR}/src/Phoenix.Sim.Lua/Phoenix.d.lua"
)
add_custom_command(TARGET PhoenixRTS2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LUA_WASM}"
        "$<TARGET_FILE_DIR:PhoenixRTS2>/lua.wasm"
    COMMENT "Copying lua.wasm"
)

add_custom_command(TARGET PhoenixRTS2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Tracy::TracyClient>
        $<TARGET_FILE_DIR:PhoenixRTS2>
    COMMENT "Copying TracyClient.dll"
)
