file(GLOB_RECURSE PHOENIXLUA_SRC "*.cpp" "*.h" "*.lua")
add_library(Phoenix.Sim.Lua ${PHOENIXLUA_SRC})
set_property(TARGET Phoenix.Sim.Lua PROPERTY FOLDER "Libraries")
target_include_directories(Phoenix.Sim.Lua PUBLIC ../../src)
if(PHX_USE_PCH)
    target_precompile_headers(Phoenix.Sim.Lua PRIVATE ../Phoenix.Sim/pch.h)
endif()
target_compile_definitions(Phoenix.Sim.Lua PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(Phoenix.Sim.Lua PUBLIC Phoenix.Sim Phoenix.Sim.Script)

# Install rules
install(TARGETS Phoenix.Sim.Lua
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/Phoenix.Sim.Lua FILES_MATCHING PATTERN "*.h")
