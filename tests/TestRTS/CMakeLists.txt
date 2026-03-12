
file(GLOB_RECURSE TESTRTS_SRC "*.cpp" "*.h" "*.inl" "*.json")

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
target_include_directories(TestRTS PUBLIC ../../src)
target_include_directories(TestRTS PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include")
target_link_libraries(TestRTS PUBLIC PhoenixSim PhoenixPhysics PhoenixSteering PhoenixLua PhoenixRTS)

# Set the target folder for organization in IDEs
set_property(TARGET TestRTS PROPERTY FOLDER "Apps")

# Set working directory to access the data files
set_property(TARGET TestRTS PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/TestRTS")

# Post-build step: copy TracyClient.dll to output directory
add_custom_command(TARGET TestRTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/vcpkg/packages/tracy_x64-windows/bin/TracyClient.dll"
        $<TARGET_FILE_DIR:TestRTS>
    COMMENT "Copying TracyClient.dll to output directory"
)