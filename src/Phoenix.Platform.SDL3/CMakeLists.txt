file(GLOB_RECURSE PLATFORM_SDL3_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Platform.SDL3 STATIC ${PLATFORM_SDL3_SOURCES})

target_include_directories(Phoenix.Platform.SDL3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Platform.SDL3 PUBLIC
    Phoenix.App
    SDL3::SDL3
)
