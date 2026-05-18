file(GLOB_RECURSE RUNTIME_SDL3_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Runtime.SDL3 STATIC ${RUNTIME_SDL3_SOURCES})

target_include_directories(Phoenix.Runtime.SDL3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Runtime.SDL3 PUBLIC
    Phoenix.Runtime
    SDL3::SDL3
)
