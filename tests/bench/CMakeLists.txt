
add_executable(PhoenixBench main.cpp)

target_compile_definitions(PhoenixBench PRIVATE PHX_PROFILE_ENABLE)

target_include_directories(PhoenixBench PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(PhoenixBench PRIVATE
    Phoenix
    Phoenix.Parallel
    Phoenix.Parallel.enkiTS
    Phoenix.Profilers.Tracy
    Phoenix.Profilers.Structured
    unofficial::sqlite3::sqlite3
)

if(MSVC)
    target_compile_options(PhoenixBench PRIVATE /W4)
endif()

set_property(TARGET PhoenixBench PROPERTY FOLDER "Tests")

# Set working directory so bench.db lands next to the exe
set_property(TARGET PhoenixBench PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
    "$<TARGET_FILE_DIR:PhoenixBench>")

# Copy Tracy DLL next to the exe (required on Windows)
add_custom_command(TARGET PhoenixBench POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Tracy::TracyClient>
        $<TARGET_FILE_DIR:PhoenixBench>
    COMMENT "Copying TracyClient.dll to PhoenixBench output directory"
)
