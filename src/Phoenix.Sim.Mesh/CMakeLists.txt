file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.Mesh STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.Mesh PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Mesh PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Mesh PUBLIC
    Phoenix.Sim
)
