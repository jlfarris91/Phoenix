file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

add_library(Phoenix.Parallel.enkiTS STATIC ${SOURCES})
set_property(TARGET Phoenix.Parallel.enkiTS PROPERTY FOLDER "Libraries/Parallel")

target_include_directories(Phoenix.Parallel.enkiTS PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Phoenix.Parallel.enkiTS PUBLIC
    Phoenix
    enkiTS::enkiTS
)
