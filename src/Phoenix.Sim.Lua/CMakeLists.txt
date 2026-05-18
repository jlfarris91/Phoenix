file(GLOB_RECURSE PHOENIXLUA_SRC "*.cpp" "*.h" "*.lua")
add_library(PhoenixLua ${PHOENIXLUA_SRC})
set_property(TARGET PhoenixLua PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixLua PUBLIC ../../src)
if(PHX_USE_PCH)
    target_precompile_headers(PhoenixLua PRIVATE ../Phoenix.Sim/pch.h)
endif()
target_compile_definitions(PhoenixLua PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(PhoenixLua PUBLIC PhoenixSim PhoenixScript)

# Install rules
install(TARGETS PhoenixLua
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixLua FILES_MATCHING PATTERN "*.h")
