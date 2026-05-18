file(GLOB_RECURSE EDITOR_IMGUI_SOURCES "*.h" "*.cpp")

add_library(PhoenixEditor.ImGui STATIC ${EDITOR_IMGUI_SOURCES})

target_include_directories(PhoenixEditor.ImGui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PhoenixEditor.ImGui PUBLIC
    PhoenixEditor.Core
    imgui::imgui
)
