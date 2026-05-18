
file(GLOB_RECURSE TESTRTS_SRC "*.cpp" "*.h" "*.inl" "*.json", "*.lua")

add_executable(TestRTS ${TESTRTS_SRC})

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
target_link_libraries(TestRTS PUBLIC Phoenix.Sim Phoenix.Sim.Physics Phoenix.Sim.Steering Phoenix.Sim.Lua Phoenix.Sim.RTS Phoenix.Sim.Script)

# Force the linker to include all object files from libs that self-register via
# static initializers (PHX_DEFINE_TYPE, IScriptBindings).  Without this, MSVC
# strips translation units whose only contribution is a static initializer, so
# ScriptBindings implementations (SimScriptBindings, RTSScriptBindings, etc.)
# are silently dropped and TypeRegistry::GetAllDerivedFrom<IScriptBindings>()
# returns nothing at runtime.
if(MSVC)
    target_link_options(TestRTS PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim>
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim.RTS>)
elseif(APPLE)
    target_link_options(TestRTS PRIVATE
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim>
        -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim.RTS>)
else()
    target_link_options(TestRTS PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:Phoenix.Sim> $<TARGET_FILE:Phoenix.Sim.RTS> -Wl,--no-whole-archive)
endif()

# Set the target folder for organization in IDEs
set_property(TARGET TestRTS PROPERTY FOLDER "Apps")

# wasm3 without TCO (MSVC doesn't guarantee tail-call elimination) uses deep
# C recursion for its op-chain interpreter.  Running Lua 5.4 through wasm3
# requires significant stack depth — Debug builds are especially heavy because
# MSVC's /RTC1 runtime checks bloat each frame.  Reserve 128 MB (virtual,
# not committed until touched).
if(MSVC)
    target_link_options(TestRTS PRIVATE "/STACK:134217728")
endif()

# ── Lua runner WASM ───────────────────────────────────────────────────────────
#
# Builds a generic Lua 5.4 interpreter as a standalone WASM binary.
# lua.wasm is shared across all worlds and loaded by FeatureLua at
# runtime.  It is placed next to the TestRTS executable so it is always
# co-located with the app regardless of working directory.
#
# Requires emcc (Emscripten) to be on PATH.  add_lua_wasm_script() is defined
# in tools/PhoenixLuaRuntime/CMakeLists.txt and will fatal-error at configure time
# if emcc is not found, ensuring the build never silently omits lua.wasm.
set(LUA_WASM "${CMAKE_CURRENT_BINARY_DIR}/lua.wasm")
add_lua_wasm_script(
    OUTPUT     "${LUA_WASM}"
    TARGET     TestRTS
    DEFS_DEST  "${CMAKE_SOURCE_DIR}/src/Phoenix.Sim.Lua/Phoenix.d.lua"
)
# Copy lua.wasm next to the TestRTS executable after each build.
add_custom_command(TARGET TestRTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LUA_WASM}"
        "$<TARGET_FILE_DIR:TestRTS>/lua.wasm"
    COMMENT "Copying lua.wasm to output directory"
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