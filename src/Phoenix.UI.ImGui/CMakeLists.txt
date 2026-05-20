file(GLOB_RECURSE UI_IMGUI_SOURCES "*.h" "*.cpp")

add_library(Phoenix.UI.ImGui STATIC ${UI_IMGUI_SOURCES})

target_include_directories(Phoenix.UI.ImGui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.UI.ImGui PUBLIC
    Phoenix.App
    imgui
)
