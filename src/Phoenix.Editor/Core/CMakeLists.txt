file(GLOB_RECURSE EDITOR_CORE_SOURCES "*.h" "*.cpp")

add_library(PhoenixEditor.Core STATIC ${EDITOR_CORE_SOURCES})

target_include_directories(PhoenixEditor.Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PhoenixEditor.Core PUBLIC
    PhoenixApp.Core
    nlohmann_json::nlohmann_json
)
