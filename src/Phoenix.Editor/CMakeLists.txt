file(GLOB_RECURSE EDITOR_CORE_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Editor STATIC ${EDITOR_CORE_SOURCES})

target_include_directories(Phoenix.Editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Editor PUBLIC
    Phoenix.App
    nlohmann_json::nlohmann_json
)
