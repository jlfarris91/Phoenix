file(GLOB_RECURSE SCENE_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Scene STATIC ${SCENE_SOURCES})

target_include_directories(Phoenix.Scene PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Scene PUBLIC
    Phoenix.Runtime
    Phoenix.Sim.ECS
)
