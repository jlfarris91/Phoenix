file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.LDS STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.LDS PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.LDS PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.LDS PUBLIC
    Phoenix.Sim
)
