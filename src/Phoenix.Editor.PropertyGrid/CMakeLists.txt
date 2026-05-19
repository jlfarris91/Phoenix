file(GLOB_RECURSE PROPERTY_GRID_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Editor.PropertyGrid STATIC ${PROPERTY_GRID_SOURCES})

target_include_directories(Phoenix.Editor.PropertyGrid PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Editor.PropertyGrid PUBLIC
    Phoenix.Editor
)
