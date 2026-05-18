file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.Blackboard STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.Blackboard PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Blackboard PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Blackboard PUBLIC
    Phoenix.Sim
)
