file(GLOB_RECURSE CORE_SOURCES "*.h" "*.cpp")

add_executable(PhoenixEditor ${CORE_SOURCES})

target_link_libraries(PhoenixEditor PRIVATE
    PhoenixEditor.ImGui
    PhoenixEditor.Core
    SDL3::SDL3
    imgui::imgui
    $<$<PLATFORM_ID:Windows>:opengl32>
    $<$<PLATFORM_ID:Linux>:GL>
    $<$<PLATFORM_ID:Darwin>:-framework OpenGL>
)

if(WIN32)
    add_custom_command(TARGET PhoenixEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:PhoenixEditor>
        COMMENT "Copying SDL3.dll"
    )
endif()

add_custom_command(TARGET PhoenixEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_layout.ini"
        "$<TARGET_FILE_DIR:PhoenixEditor>/default_layout.ini"
    COMMENT "Copying default_layout.ini"
)

add_custom_command(TARGET PhoenixEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:PhoenixEditor>/data"
    COMMENT "Copying data directory"
)
