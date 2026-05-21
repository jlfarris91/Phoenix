function(phx_add_game TARGET)
    cmake_parse_arguments(GAME "" "" "SOURCES;DATA" ${ARGN})

    set(GENERATED_MAIN "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_main.cpp")
    file(WRITE "${GENERATED_MAIN}"
        "#include \"PhoenixMain.h\"\n"
        "int main(int argc, char** argv) { return Phoenix::PhoenixMain(argc, argv); }\n"
    )

    add_executable(${TARGET} ${GENERATED_MAIN} ${GAME_SOURCES})

    target_include_directories(${TARGET} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(${TARGET} PRIVATE
        Phoenix.Engine
        Phoenix.Platform.SDL3
        Phoenix.UI.ImGui.SDL3
        Phoenix.Renderer.SDL3
    )

    set_property(TARGET ${TARGET} PROPERTY FOLDER "Apps")
    set_property(TARGET ${TARGET} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    if(WIN32)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "Copying SDL3.dll"
        )
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/data"
                "$<TARGET_FILE_DIR:${TARGET}>/data"
            COMMENT "Copying data"
        )
    endif()
endfunction()
