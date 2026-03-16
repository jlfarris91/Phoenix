file(GLOB_RECURSE PHOENIXLUA_SRC "*.cpp" "*.h")
find_package(sol2 CONFIG REQUIRED)
find_package(lua CONFIG REQUIRED)
add_library(PhoenixLua ${PHOENIXLUA_SRC})
set_property(TARGET PhoenixLua PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixLua PUBLIC "${CMAKE_SOURCE_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include" ../../src)
if(PHX_USE_PCH)
    target_precompile_headers(PhoenixLua PRIVATE ../PhoenixSim/pch.h)
endif()
target_compile_definitions(PhoenixLua PUBLIC PHX_PROFILE_ENABLE)
target_link_libraries(PhoenixLua PUBLIC PhoenixSim PhoenixPhysics)

# Install rules
install(TARGETS PhoenixLua
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixLua FILES_MATCHING PATTERN "*.h")
