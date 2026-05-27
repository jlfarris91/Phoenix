file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

add_library(Phoenix.Parallel STATIC ${SOURCES})
set_property(TARGET Phoenix.Parallel PROPERTY FOLDER "Libraries/Parallel")

target_include_directories(Phoenix.Parallel PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Parallel PUBLIC
    Phoenix
)
