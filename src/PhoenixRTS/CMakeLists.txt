file(GLOB_RECURSE PHOENIXRTS_SRC "*.cpp" "*.h")
add_library(PhoenixRTS ${PHOENIXRTS_SRC})
set_property(TARGET PhoenixRTS PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixRTS PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include" ../../src)
target_precompile_headers(PhoenixRTS PRIVATE ../PhoenixSim/pch.h)
target_compile_definitions(PhoenixRTS PUBLIC PHX_PROFILE_ENABLE)
target_compile_options(PhoenixRTS PRIVATE "/MP")
target_link_libraries(PhoenixRTS PUBLIC PhoenixSim PhoenixPhysics PhoenixSteering)

# Install rules
install(TARGETS PhoenixRTS
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixRTS FILES_MATCHING PATTERN "*.h")
