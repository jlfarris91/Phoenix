cmake_minimum_required(VERSION 3.25)

# Auto-install vcpkg if not found
if(NOT DEFINED VCPKG_ROOT)
    set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
endif()
if(NOT EXISTS "${VCPKG_ROOT}/vcpkg.exe")
    message(STATUS "vcpkg not found, installing...")
    execute_process(COMMAND git clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}")
    execute_process(COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
endif()
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(Phoenix LANGUAGES C CXX)

include(cmake/PhoenixGame.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseWithSymbols" CACHE STRING "Configs" FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release,ReleaseWithSymbols>:NDEBUG>
)

set(CMAKE_MAP_IMPORTED_CONFIG_RELEASEWITHSYMBOLS RELEASE)
set(CMAKE_CXX_FLAGS_RELEASEWITHSYMBOLS "${CMAKE_CXX_FLAGS_RELEASE} /Zi" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASEWITHSYMBOLS "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG" CACHE STRING "" FORCE)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4101>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-unused-parameter>
    $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<NOT:$<BOOL:${EMSCRIPTEN}>>>:-Wno-changes-meaning>
    $<$<CXX_COMPILER_ID:GNU>:-fpermissive>
)

set(CMAKE_VS_DEBUGGER_VISUALIZER_FILES "${CMAKE_SOURCE_DIR}/tools/PhoenixSim.natvis")

# ── Options ───────────────────────────────────────────────────────────────────

option(PHX_USE_PCH           "Use precompiled headers"                      ON)
option(PHX_BUILD_TESTAPP     "Build PhoenixRTS demo application"            ON)
option(PHX_BUILD_TESTS       "Build unit tests"                             OFF)
option(PHX_BUILD_ENGINE      "Build PhoenixEngine and game projects"        OFF)
option(PE_BUILD_STANDALONE   "Build the standalone Phoenix Editor app"      OFF)

# ── Packages ──────────────────────────────────────────────────────────────────

find_package(nlohmann_json CONFIG REQUIRED)

if(NOT EMSCRIPTEN)
    find_package(inja CONFIG REQUIRED)
endif()

if(PHX_BUILD_TESTAPP OR PE_BUILD_STANDALONE OR PHX_BUILD_ENGINE)
    find_package(SDL3  CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    find_package(glm   CONFIG REQUIRED)
    find_package(EnTT  CONFIG REQUIRED)
endif()

if(PHX_BUILD_TESTS)
    find_package(doctest CONFIG REQUIRED)
endif()

# ── Core libraries ────────────────────────────────────────────────────────────

add_subdirectory(src/Phoenix)
add_subdirectory(src/Phoenix.Sim)

# ── Phoenix.Sim feature libraries ─────────────────────────────────────────────

add_subdirectory(src/Phoenix.Sim.Blackboard)
add_subdirectory(src/Phoenix.Sim.Strings)
add_subdirectory(src/Phoenix.Sim.Debug)
add_subdirectory(src/Phoenix.Sim.Mesh)
add_subdirectory(src/Phoenix.Sim.Nav)
add_subdirectory(src/Phoenix.Sim.Tasks)
add_subdirectory(src/Phoenix.Sim.ECS)
add_subdirectory(src/Phoenix.Sim.LDS)

# ── Domain simulation libraries ───────────────────────────────────────────────

add_subdirectory(src/Phoenix.Sim.Physics)
add_subdirectory(src/Phoenix.Sim.Steering)
add_subdirectory(src/Phoenix.Sim.RTS)
add_subdirectory(src/Phoenix.Sim.Script)
add_subdirectory(src/Phoenix.Sim.Lua)

# ── App / Editor / Runtime framework ─────────────────────────────────────────

add_subdirectory(src/Phoenix.App)
add_subdirectory(src/Phoenix.Editor)
add_subdirectory(src/Phoenix.Runtime)

add_subdirectory(src/Phoenix.Editor.PropertyGrid)
add_subdirectory(src/Phoenix.Scene)

if(NOT EMSCRIPTEN)
    add_subdirectory(src/Phoenix.Math)
    add_subdirectory(src/Phoenix.Renderer)
endif()

if(PHX_BUILD_ENGINE OR PE_BUILD_STANDALONE)
    add_subdirectory(src/Phoenix.UI.ImGui)
    add_subdirectory(src/Phoenix.App.Dev)
endif()

if(PHX_BUILD_ENGINE)
    add_subdirectory(src/Phoenix.Engine)
endif()

if(PE_BUILD_STANDALONE)
    add_subdirectory(src/Phoenix.Editor.ImGui)
    add_subdirectory(apps/PhoenixEditor)
endif()

# ── Build tools (code generators — not needed for WASM builds) ───────────────

if(NOT EMSCRIPTEN)
    add_subdirectory(src/Phoenix.Build.APIGen)
    add_subdirectory(src/Phoenix.Build.WasmGen)
    add_subdirectory(src/Phoenix.Build.LuaGen)
    add_subdirectory(build/LuaRuntime)
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────

if(PHX_BUILD_TESTAPP AND NOT EMSCRIPTEN)
    add_subdirectory(apps/PhoenixRTS)
endif()

if(PHX_BUILD_ENGINE)
    add_subdirectory(apps/PhoenixEngine)
    add_subdirectory(apps/PhoenixRTS2)
endif()

if(PHX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/unit)
    add_subdirectory(tests/Phoenix.Editor)
endif()
