file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.Nav STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.Nav PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Nav PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Nav PUBLIC
    Phoenix.Sim
    Phoenix.Sim.Mesh
)
