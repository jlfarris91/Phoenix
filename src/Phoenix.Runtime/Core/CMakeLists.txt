file(GLOB_RECURSE RUNTIME_CORE_SOURCES "*.h" "*.cpp")

add_library(PhoenixRuntime.Core STATIC ${RUNTIME_CORE_SOURCES})

target_include_directories(PhoenixRuntime.Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PhoenixRuntime.Core PUBLIC
    PhoenixApp.Core
)
