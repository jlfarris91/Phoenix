file(GLOB_RECURSE APP_CORE_SOURCES "*.h" "*.cpp")

add_library(Phoenix.App STATIC ${APP_CORE_SOURCES})

target_include_directories(Phoenix.App PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.App PUBLIC
    Phoenix.Reflection
)
