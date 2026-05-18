file(GLOB_RECURSE RUNTIME_SDL3_SOURCES "*.h" "*.cpp")

add_library(PhoenixRuntime.SDL3 STATIC ${RUNTIME_SDL3_SOURCES})

target_include_directories(PhoenixRuntime.SDL3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PhoenixRuntime.SDL3 PUBLIC
    PhoenixRuntime.Core
    SDL3::SDL3
)
