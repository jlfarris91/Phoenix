file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.Debug STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.Debug PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Debug PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Debug PUBLIC
    Phoenix.Sim
)
