file(GLOB_RECURSE APP_CORE_SOURCES "*.h" "*.cpp")

add_library(PhoenixApp.Core STATIC ${APP_CORE_SOURCES})

target_include_directories(PhoenixApp.Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PhoenixApp.Core PUBLIC
    Phoenix.Reflection
)
