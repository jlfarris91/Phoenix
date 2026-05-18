file(GLOB_RECURSE PHOENIXSIM_SRC "*.cpp" "*.h")
find_package(nlohmann_json CONFIG REQUIRED)
add_library(PhoenixSim ${PHOENIXSIM_SRC})
set_property(TARGET PhoenixSim PROPERTY FOLDER "Libraries")
target_include_directories(PhoenixSim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
target_link_libraries(PhoenixSim PUBLIC nlohmann_json)
if(PHX_USE_PCH)
    target_precompile_headers(PhoenixSim PRIVATE pch.h)
endif()
target_compile_definitions(PhoenixSim PUBLIC PHX_PROFILE_ENABLE)

# Install rules
install(TARGETS PhoenixSim
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/PhoenixSim FILES_MATCHING PATTERN "*.h")
