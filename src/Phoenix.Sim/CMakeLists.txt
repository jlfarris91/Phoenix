file(GLOB_RECURSE PHOENIXSIM_SRC "*.cpp" "*.h")
find_package(nlohmann_json CONFIG REQUIRED)
add_library(Phoenix.Sim ${PHOENIXSIM_SRC})
set_property(TARGET Phoenix.Sim PROPERTY FOLDER "Libraries")
target_include_directories(Phoenix.Sim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
target_link_libraries(Phoenix.Sim PUBLIC nlohmann_json)
if(PHX_USE_PCH)
    target_precompile_headers(Phoenix.Sim PRIVATE pch.h)
endif()
target_compile_definitions(Phoenix.Sim PUBLIC PHX_PROFILE_ENABLE)

# Install rules
install(TARGETS Phoenix.Sim
    RUNTIME DESTINATION bin      # .exe and .dll (Windows)
    LIBRARY DESTINATION lib      # .so (Unix)
    ARCHIVE DESTINATION lib      # .lib (Windows static)
)
install(DIRECTORY . DESTINATION include/Phoenix.Sim FILES_MATCHING PATTERN "*.h")
