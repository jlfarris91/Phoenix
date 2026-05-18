file(GLOB_RECURSE PHOENIXRTS_SRC "*.cpp" "*.h")
add_library(Phoenix.Sim.RTS ${PHOENIXRTS_SRC})
set_property(TARGET Phoenix.Sim.RTS PROPERTY FOLDER "Libraries")
target_include_directories(Phoenix.Sim.RTS PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
if(PHX_USE_PCH)
    target_precompile_headers(Phoenix.Sim.RTS PRIVATE ../Phoenix.Sim/pch.h)
endif()
target_compile_definitions(Phoenix.Sim.RTS PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(Phoenix.Sim.RTS PUBLIC
    Phoenix.Sim
    Phoenix.Sim.Tasks
    Phoenix.Sim.ECS
    Phoenix.Sim.LDS
    Phoenix.Sim.Physics
    Phoenix.Sim.Steering
)

# Install rules
install(TARGETS Phoenix.Sim.RTS
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/Phoenix.Sim.RTS FILES_MATCHING PATTERN "*.h")
