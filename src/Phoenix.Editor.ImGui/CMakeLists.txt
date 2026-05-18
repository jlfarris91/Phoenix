file(GLOB_RECURSE EDITOR_IMGUI_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Editor.ImGui STATIC ${EDITOR_IMGUI_SOURCES})

target_include_directories(Phoenix.Editor.ImGui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Editor.ImGui PUBLIC
    Phoenix.Editor
    imgui::imgui
)
