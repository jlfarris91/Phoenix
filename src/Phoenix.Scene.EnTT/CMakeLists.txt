file(GLOB_RECURSE SOURCES "*.h" "*.cpp")

add_library(Phoenix.Scene.EnTT STATIC ${SOURCES})
set_property(TARGET Phoenix.Scene.EnTT PROPERTY FOLDER "Libraries")

target_include_directories(Phoenix.Scene.EnTT PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(EnTT CONFIG REQUIRED)

target_link_libraries(Phoenix.Scene.EnTT PUBLIC
    Phoenix.Scene
    EnTT::EnTT
)
