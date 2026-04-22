file(GLOB_RECURSE PHOENIXSTEERING_SRC "*.cpp" "*.h")
add_library(PhoenixSteering ${PHOENIXSTEERING_SRC})
set_property(TARGET PhoenixSteering PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixSteering PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include" ../../src)
if(PHX_USE_PCH)
    target_precompile_headers(PhoenixSteering PRIVATE ../PhoenixSim/pch.h)
endif()
target_compile_definitions(PhoenixSteering PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(PhoenixSteering PUBLIC PhoenixSim PhoenixPhysics)
# Heavy template instantiation in job types can overflow MSVC's debug-info linker pass.
# /DEBUG:FASTLINK avoids merging all type info into one PDB pass, preventing LNK1000.
if(MSVC)
    target_link_options(PhoenixSteering PRIVATE $<$<CONFIG:Debug>:/DEBUG:FASTLINK>)
endif()

# Install rules
install(TARGETS PhoenixSteering
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixSteering FILES_MATCHING PATTERN "*.h")
