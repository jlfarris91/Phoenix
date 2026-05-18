file(GLOB_RECURSE PHOENIX_REFLECTION_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Reflection STATIC ${PHOENIX_REFLECTION_SOURCES})

target_include_directories(Phoenix.Reflection PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
