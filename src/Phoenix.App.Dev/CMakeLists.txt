find_package(EnTT CONFIG REQUIRED)

file(GLOB_RECURSE APP_DEV_SOURCES "*.h" "*.cpp")

add_library(Phoenix.App.Dev STATIC ${APP_DEV_SOURCES})

target_include_directories(Phoenix.App.Dev PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.App.Dev PUBLIC
    Phoenix.App
    Phoenix.Renderer
    Phoenix.Scene
    Phoenix.UI.ImGui
    EnTT::EnTT
    SDL3::SDL3
    imgui::imgui
)
