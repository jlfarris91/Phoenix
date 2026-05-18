add_library(Phoenix.Sim.Mesh INTERFACE)
set_property(TARGET Phoenix.Sim.Mesh PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Mesh INTERFACE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Mesh INTERFACE
    Phoenix.Sim
)
