file(GLOB_RECURSE RUNTIME_CORE_SOURCES "*.h" "*.cpp")

add_library(Phoenix.Runtime STATIC ${RUNTIME_CORE_SOURCES})

target_include_directories(Phoenix.Runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Phoenix.Runtime PUBLIC
    Phoenix.App
    Phoenix.Sim
)
