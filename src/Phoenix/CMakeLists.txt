file(GLOB_RECURSE PHOENIX_SRC "*.h" "*.cpp")

add_library(Phoenix STATIC ${PHOENIX_SRC})
set_property(TARGET Phoenix PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
