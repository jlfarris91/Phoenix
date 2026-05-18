file(GLOB_RECURSE PHOENIXPHYSICS_SRC "*.cpp" "*.h")
add_library(PhoenixPhysics ${PHOENIXPHYSICS_SRC})
set_property(TARGET PhoenixPhysics PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixPhysics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
if(PHX_USE_PCH)
    target_precompile_headers(PhoenixPhysics PRIVATE ../Phoenix.Sim/pch.h)
endif()
target_compile_definitions(PhoenixPhysics PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(PhoenixPhysics PUBLIC PhoenixSim)
# Heavy template instantiation in job types can overflow MSVC's debug-info linker pass.
if(MSVC)
    target_link_options(PhoenixPhysics PRIVATE $<$<CONFIG:Debug>:/DEBUG:FASTLINK>)
endif()

# Install rules
install(TARGETS PhoenixPhysics
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixPhysics FILES_MATCHING PATTERN "*.h")
