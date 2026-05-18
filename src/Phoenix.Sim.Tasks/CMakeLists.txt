file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.Tasks STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.Tasks PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Tasks PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Tasks PUBLIC
    Phoenix.Sim
)
