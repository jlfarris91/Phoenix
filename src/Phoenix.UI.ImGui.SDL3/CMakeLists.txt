file(GLOB_RECURSE UI_IMGUI_SDL3_SOURCES "*.h" "*.cpp")

add_library(Phoenix.UI.ImGui.SDL3 STATIC ${UI_IMGUI_SDL3_SOURCES})

target_include_directories(Phoenix.UI.ImGui.SDL3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.UI.ImGui.SDL3 PUBLIC
    Phoenix.UI.ImGui
    Phoenix.Platform.SDL3
    imgui
)
