file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.ECS STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.ECS PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.ECS PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.ECS PUBLIC
    Phoenix.Sim
    Phoenix.Sim.Blackboard
    Phoenix.Sim.Tasks
)
