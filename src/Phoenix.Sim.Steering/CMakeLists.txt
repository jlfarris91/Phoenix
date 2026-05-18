file(GLOB_RECURSE PHOENIXSTEERING_SRC "*.cpp" "*.h")
add_library(Phoenix.Sim.Steering ${PHOENIXSTEERING_SRC})
set_property(TARGET Phoenix.Sim.Steering PROPERTY FOLDER "Libraries")
target_include_directories(Phoenix.Sim.Steering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
if(PHX_USE_PCH)
    target_precompile_headers(Phoenix.Sim.Steering PRIVATE ../Phoenix.Sim/pch.h)
endif()
target_compile_definitions(Phoenix.Sim.Steering PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(Phoenix.Sim.Steering PUBLIC Phoenix.Sim Phoenix.Sim.Physics)
# Heavy template instantiation in job types can overflow MSVC's debug-info linker pass.
# /DEBUG:FASTLINK avoids merging all type info into one PDB pass, preventing LNK1000.
if(MSVC)
    target_link_options(Phoenix.Sim.Steering PRIVATE $<$<CONFIG:Debug>:/DEBUG:FASTLINK>)
endif()

# Install rules
install(TARGETS Phoenix.Sim.Steering
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/Phoenix.Sim.Steering FILES_MATCHING PATTERN "*.h")
