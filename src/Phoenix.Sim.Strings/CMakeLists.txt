file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.inl")

add_library(Phoenix.Sim.Strings STATIC ${SOURCES})
set_property(TARGET Phoenix.Sim.Strings PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Sim.Strings PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Sim.Strings PUBLIC
    Phoenix.Sim
)
