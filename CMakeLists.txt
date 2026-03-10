

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

# Apply global settings for custom configs
foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
	if(config MATCHES "ReleaseWithSymbols")
		set(CMAKE_CXX_FLAGS_${config} "${CMAKE_CXX_FLAGS_${config}} /Zi /O2")
		add_compile_definitions($<$<CONFIG:${config}>:NDEBUG>)
	endif()
endforeach()

# Set natvis visualizer for all targets
set(CMAKE_VS_DEBUGGER_VISUALIZER_FILES "${CMAKE_SOURCE_DIR}/tools/PhoenixSim.natvis")

find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(src/PhoenixSim)
add_subdirectory(src/PhoenixPhysics)
add_subdirectory(src/PhoenixSteering)
add_subdirectory(src/PhoenixLua)
add_subdirectory(src/PhoenixRTS)
add_subdirectory(tests/TestRTS)
