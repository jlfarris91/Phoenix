file(GLOB_RECURSE RENDERER_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Renderer STATIC ${RENDERER_SOURCES})

target_include_directories(Phoenix.Renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Renderer PUBLIC
    Phoenix.App
    glm::glm
)
