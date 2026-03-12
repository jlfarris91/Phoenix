

# Auto-install vcpkg if not found
if(NOT DEFINED VCPKG_ROOT)
	set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
endif()
if(NOT EXISTS "${VCPKG_ROOT}/vcpkg.exe")
	message(STATUS "vcpkg not found, installing...")
	execute_process(COMMAND git clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}")
	execute_process(COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
endif()

# Use vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

cmake_minimum_required(VERSION 3.20)
project(Phoenix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseWithSymbols" CACHE STRING "Configs" FORCE)

# The Runtime Library (/MD vs /MDd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Definitions
add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release,ReleaseWithSymbols>:NDEBUG>
)

# Tells CMake that for the project's 'ReleaseWithSymbols' configuration,
# it should use the 'Release' version of any imported targets.
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASEWITHSYMBOLS RELEASE)
set(CMAKE_CXX_FLAGS_RELEASEWITHSYMBOLS "${CMAKE_CXX_FLAGS_RELEASE} /Zi" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASEWITHSYMBOLS "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG" CACHE STRING "" FORCE)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4101>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-unused-parameter>
)

# Set natvis visualizer for all targets
set(CMAKE_VS_DEBUGGER_VISUALIZER_FILES "${CMAKE_SOURCE_DIR}/tools/PhoenixSim.natvis")

find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(src/PhoenixSim)
add_subdirectory(src/PhoenixPhysics)
add_subdirectory(src/PhoenixSteering)
add_subdirectory(src/PhoenixLua)
add_subdirectory(src/PhoenixRTS)
add_subdirectory(tests/TestRTS)
