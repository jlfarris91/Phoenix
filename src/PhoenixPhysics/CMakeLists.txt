file(GLOB_RECURSE PHOENIXPHYSICS_SRC "*.cpp" "*.h")
add_library(PhoenixPhysics ${PHOENIXPHYSICS_SRC})
set_property(TARGET PhoenixPhysics PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixPhysics PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include" ../../src)
target_precompile_headers(PhoenixPhysics PRIVATE ../PhoenixSim/pch.h)
target_compile_definitions(PhoenixPhysics PUBLIC PHX_PROFILE_ENABLE)
target_compile_options(PhoenixPhysics PRIVATE "/MP")
target_link_libraries(PhoenixPhysics PUBLIC PhoenixSim)

# Install rules
install(TARGETS PhoenixPhysics
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixPhysics FILES_MATCHING PATTERN "*.h")
