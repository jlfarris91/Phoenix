file(GLOB_RECURSE ENGINE_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Engine STATIC ${ENGINE_SOURCES})

target_include_directories(Phoenix.Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Engine PUBLIC
    Phoenix.App
    Phoenix.Runtime
)
