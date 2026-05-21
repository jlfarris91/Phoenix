file(GLOB_RECURSE UI_IMGUI_SOURCES "*.h")

add_library(Phoenix.UI.ImGui INTERFACE)

target_include_directories(Phoenix.UI.ImGui INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.UI.ImGui INTERFACE
    Phoenix.App
    imgui::imgui
)
