file(GLOB_RECURSE MATH_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Math STATIC ${MATH_SOURCES})

target_include_directories(Phoenix.Math PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Math PUBLIC
    glm::glm
)
