file(GLOB_RECURSE CORE_SOURCES "*.h" "*.cpp")

add_executable(Phoenix.Editor.App ${CORE_SOURCES})
set_target_properties(Phoenix.Editor.App PROPERTIES OUTPUT_NAME "PhoenixEditor")

target_link_libraries(Phoenix.Editor.App PRIVATE
    Phoenix.Editor.ImGui
    Phoenix.Editor
    SDL3::SDL3
    imgui::imgui
    $<$<PLATFORM_ID:Windows>:opengl32>
    $<$<PLATFORM_ID:Linux>:GL>
    $<$<PLATFORM_ID:Darwin>:-framework OpenGL>
)

if(WIN32)
    add_custom_command(TARGET Phoenix.Editor.App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:Phoenix.Editor.App>
        COMMENT "Copying SDL3.dll"
    )
endif()

add_custom_command(TARGET Phoenix.Editor.App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_layout.ini"
        "$<TARGET_FILE_DIR:Phoenix.Editor.App>/default_layout.ini"
    COMMENT "Copying default_layout.ini"
)

add_custom_command(TARGET Phoenix.Editor.App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:Phoenix.Editor.App>/data"
    COMMENT "Copying data directory"
)
