
file(GLOB_RECURSE TESTRTS_SRC "*.cpp" "*.h" "*.inl" "*.json")
find_package(tracy CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(lua CONFIG REQUIRED)
add_executable(TestRTS ${TESTRTS_SRC})

target_include_directories(TestRTS PUBLIC ../../src)
target_compile_definitions(TestRTS PUBLIC TRACY_ENABLE PHX_PROFILE_ENABLE IMGUI_DEFINE_MATH_OPERATORS)
find_package(imgui CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(TestRTS PUBLIC imgui::imgui SDL3::SDL3 Tracy::TracyClient nlohmann_json PhoenixSim PhoenixPhysics PhoenixSteering PhoenixLua PhoenixRTS)
target_include_directories(TestRTS PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include")

# Set the target folder for organization in IDEs
set_property(TARGET TestRTS PROPERTY FOLDER "Apps")

# Set working directory to access the data files
set_property(TARGET TestRTS PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/TestRTS")

# Post-build step: copy TracyClient.dll to output directory
add_custom_command(TARGET TestRTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/bin/TracyClient.dll"
        $<TARGET_FILE_DIR:TestRTS>
    COMMENT "Copying TracyClient.dll to output directory"
)
