
file(GLOB_RECURSE TESTRTS_SRC "*.cpp" "*.h" "*.inl" "*.json", "*.lua")

add_executable(TestRTS ${TESTRTS_SRC})

find_package(sol2 CONFIG REQUIRED)
target_link_libraries(TestRTS PRIVATE sol2::sol2)

find_package(Lua REQUIRED)
target_include_directories(TestRTS PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(TestRTS PRIVATE ${LUA_LIBRARIES})

find_package(imgui CONFIG REQUIRED)
target_link_libraries(TestRTS PRIVATE imgui::imgui)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(TestRTS PRIVATE SDL3::SDL3)

find_package(Tracy CONFIG REQUIRED)
target_link_libraries(TestRTS PRIVATE Tracy::TracyClient)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(TestRTS PRIVATE nlohmann_json::nlohmann_json)

target_compile_definitions(TestRTS PUBLIC TRACY_ENABLE PHX_PROFILE_ENABLE IMGUI_DEFINE_MATH_OPERATORS)
if(MSVC)
    target_compile_options(TestRTS PRIVATE /bigobj)
endif()
target_include_directories(TestRTS PUBLIC ../../src)
target_include_directories(TestRTS PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include")
target_link_libraries(TestRTS PUBLIC PhoenixSim PhoenixPhysics PhoenixSteering PhoenixLua PhoenixRTS PhoenixScript)

# Set the target folder for organization in IDEs
set_property(TARGET TestRTS PROPERTY FOLDER "Apps")

# wasm3 without TCO (MSVC doesn't guarantee tail-call elimination) uses deep
# C recursion for its op-chain interpreter.  Running a Lua 5.4 script through
# wasm3 can easily exhaust the default 1 MB thread stack.  Reserve 32 MB.
if(MSVC)
    target_link_options(TestRTS PRIVATE "/STACK:33554432")
endif()

# ── Lua runner WASM ───────────────────────────────────────────────────────────
#
# Builds a generic Lua 5.4 interpreter as a standalone WASM binary.
# LuaRunner.wasm is shared across all worlds and loaded by FeatureLua at
# runtime.  It is placed next to the TestRTS executable so it is always
# co-located with the app regardless of working directory.
#
# Requires emcc (Emscripten) to be on PATH.  add_lua_wasm_script() is defined
# in src/PhoenixLuaWasm/CMakeLists.txt and will fatal-error at configure time
# if emcc is not found, ensuring the build never silently omits LuaRunner.wasm.
set(LUARUNNER_WASM "${CMAKE_CURRENT_BINARY_DIR}/LuaRunner.wasm")
add_lua_wasm_script(
    OUTPUT  "${LUARUNNER_WASM}"
    TARGET  TestRTS
)
# Copy LuaRunner.wasm next to the TestRTS executable after each build.
add_custom_command(TARGET TestRTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LUARUNNER_WASM}"
        "$<TARGET_FILE_DIR:TestRTS>/LuaRunner.wasm"
    COMMENT "Copying LuaRunner.wasm to output directory"
)

# Set working directory to access the data files
set_property(TARGET TestRTS PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/TestRTS")

# Post-build step: copy TracyClient.dll to output directory
add_custom_command(TARGET TestRTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Tracy::TracyClient>
        $<TARGET_FILE_DIR:TestRTS>
    COMMENT "Copying TracyClient.dll to output directory"
)