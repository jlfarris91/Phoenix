file(GLOB_RECURSE RENDERER_SDL3_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Renderer.SDL3 STATIC ${RENDERER_SDL3_SOURCES})

target_include_directories(Phoenix.Renderer.SDL3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Renderer.SDL3 PUBLIC
    Phoenix.Renderer
    SDL3::SDL3
)
