file(GLOB_RECURSE PHOENIXPHYSICS_SRC "*.cpp" "*.h")
add_library(Phoenix.Sim.Physics ${PHOENIXPHYSICS_SRC})
set_property(TARGET Phoenix.Sim.Physics PROPERTY FOLDER "Libraries")
target_include_directories(Phoenix.Sim.Physics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
if(PHX_USE_PCH)
    target_precompile_headers(Phoenix.Sim.Physics PRIVATE ../Phoenix.Sim/pch.h)
endif()
target_compile_definitions(Phoenix.Sim.Physics PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(Phoenix.Sim.Physics PUBLIC Phoenix.Sim)
# Heavy template instantiation in job types can overflow MSVC's debug-info linker pass.
if(MSVC)
    target_link_options(Phoenix.Sim.Physics PRIVATE $<$<CONFIG:Debug>:/DEBUG:FASTLINK>)
endif()

# Install rules
install(TARGETS Phoenix.Sim.Physics
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/Phoenix.Sim.Physics FILES_MATCHING PATTERN "*.h")
