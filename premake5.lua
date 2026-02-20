-- premake5.lua

local projects = "" .. _MAIN_SCRIPT_DIR .. "/.build/" .. _ACTION
local src = "" .. _MAIN_SCRIPT_DIR .. "/src"
local ext = _MAIN_SCRIPT_DIR .. "/ext"

workspace "Phoenix"
    platforms { "x64" }
    configurations { "Debug", "Release", "ReleaseWithSymbols" }
    startproject "TestApp"
    warnings "default"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    debugdir (_MAIN_SCRIPT_DIR)
    location (_MAIN_SCRIPT_DIR)
    buildoptions { "/MP" }

    group "External"
        project "lua"
        project "sol2"

    group "Phoenix"
        project "PhoenixSim"
        project "PhoenixPhysics"
        project "PhoenixSteering"
        project "PhoenixLua"
        project "PhoenixRTS"

    group "Tests"
        project "TestRTS"

    group ""

project "lua"
    kind "StaticLib"
    location (projects)

    files { 
        ext .. "/lua/lua-5.4.8/src/**"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"
        
    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "sol2"
    kind "StaticLib"
    location (projects)

    dependson ( "lua" )

    files { 
        ext .. "/sol/**"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"
        
    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "PhoenixSim"
    kind "StaticLib"
    location (projects)

    files { src .. "/PhoenixSim/**", }
    includedirs { src }

    pchheader "PhoenixSim/pch.h"
    pchsource (src .. "/PhoenixSim/pch.cpp")
    buildoptions { "/FIPhoenixSim/pch.h" }

    externalincludedirs {
        ext,
        ext .. "/nlohmann/*",
    }

    -- defines { "PHOENIX_DLL" }
    -- defines { "PHOENIXCORE_DLL_EXPORTS" }
    defines { "PHX_PROFILE_ENABLE" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"
        
    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "PhoenixPhysics"
    kind "StaticLib"
    location (projects)

    dependson { "PhoenixSim" }

    -- defines { "PHOENIX_DLL" }
    -- defines { "PHOENIX_PHYSICS_DLL_EXPORTS" }
    defines { "PHX_PROFILE_ENABLE" }

    files { src .. "/PhoenixPhysics/**" }
    includedirs { src }

    pchheader "PhoenixSim/pch.h"
    pchsource (src .. "/PhoenixPhysics/pch.cpp")
    buildoptions { "/FIPhoenixSim/pch.h" }

    externalincludedirs {
        ext,
        ext .. "/nlohmann/*",
    }

    links { "PhoenixSim" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"

    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "PhoenixSteering"
    kind "StaticLib"
    location (projects)

    dependson { "PhoenixSim", "PhoenixPhysics" }

    -- defines { "PHOENIX_DLL" }
    -- defines { "PHOENIX_STEERING_DLL_EXPORTS" }
    defines { "PHX_PROFILE_ENABLE" }

    files { src .. "/PhoenixSteering/**" }
    includedirs { src }

    pchheader "PhoenixSim/pch.h"
    pchsource (src .. "/PhoenixSteering/pch.cpp")
    buildoptions { "/FIPhoenixSim/pch.h" }

    externalincludedirs {
        ext,
        ext .. "/nlohmann/*",
    }

    links { "PhoenixSim" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"

    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "PhoenixLua"
    kind "StaticLib"
    location (projects)

    dependson {
        "lua",
        "sol2",
        "PhoenixSim",
        "PhoenixPhysics"
    }

    -- defines { "PHOENIX_DLL" }
    -- defines { "PHOENIXSIM_DLL_EXPORTS" }
    defines { "PHX_PROFILE_ENABLE" }

    files { src .. "/PhoenixLua/**" }
    includedirs { src }

    pchheader "PhoenixSim/pch.h"
    pchsource (src .. "/PhoenixLua/pch.cpp")
    buildoptions { "/FIPhoenixSim/pch.h" }

    externalincludedirs {
        ext,
        ext .. "/nlohmann/*",
        ext .. "/lua/lua-5.4.8/src/"
    }

    links {
        "lua",
        "PhoenixSim",
        "PhoenixPhysics",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"
        
    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "PhoenixRTS"
    kind "StaticLib"
    location (projects)

    dependson {
        "PhoenixSim",
        "PhoenixPhysics",
        "PhoenixSteering",
    }

    -- defines { "PHOENIX_DLL" }
    -- defines { "PHOENIX_RTS_DLL_EXPORTS" }
    defines { "PHX_PROFILE_ENABLE" }

    files { src .. "/PhoenixRTS/**" }
    includedirs { src }

    pchheader "PhoenixSim/pch.h"
    pchsource (src .. "/PhoenixRTS/pch.cpp")
    buildoptions { "/FIPhoenixSim/pch.h" }

    externalincludedirs {
        ext,
        ext .. "/nlohmann/*"
    }

    links {
        "PhoenixSim",
        "PhoenixPhysics",
        "PhoenixSteering",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"
        
    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }

project "TestRTS"
    kind "ConsoleApp"
    location (projects)
    targetdir (projects .. "/bin/%{cfg.platform}/%{cfg.buildcfg}/TestRTS")
    debugdir (_MAIN_SCRIPT_DIR .. "/tests/TestRTS")

    dependson {
        "PhoenixSim",
        "PhoenixPhysics",
        "PhoenixSteering",
        "PhoenixLua",
        "PhoenixRTS"
    }

    -- defines { "PHOENIX_DLL" }
    defines { "TRACY_ENABLE", "PHX_PROFILE_ENABLE" }

    files {
        "tests/TestRTS/**.h",
        "tests/TestRTS/**.inl",
        "tests/TestRTS/**.cpp",
        "tests/TestRTS/**.json",

        ext .. "/imgui/*",
        ext .. "/imgui/backends/imgui_impl_sdl3.h",
        ext .. "/imgui/backends/imgui_impl_sdl3.cpp",
        ext .. "/imgui/backends/imgui_impl_sdlrenderer3.h",
        ext .. "/imgui/backends/imgui_impl_sdlrenderer3.cpp",

        ext .. "/tracy/TracyClient.cpp",
    }

    externalincludedirs {
        src,
        ext,
        ext .. "/imgui/",
        ext .. "/imgui/**",
        ext .. "/nlohmann/*",
        ext .. "/lua/lua-5.4.8/src/",
        ext .. "/tracy/"
    }

    libdirs {
        ext .. "/SDL3/x64/Debug"
    }

    links {
        "lua",
        "PhoenixSim",
        "PhoenixPhysics",
        "PhoenixSteering",
        "PhoenixLua",
        "PhoenixRTS",
        "SDL3"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "Off"
        optimize "speed"

    filter "configurations:ReleaseWithSymbols"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "On"
        optimize "speed"
        
    filter {}

    -- TODO (jfarris): fix
    disablewarnings {
        "4251", "4275"
    }
    
    filter "not configurations:ReleaseWithSymbols"
        postbuildcommands {
            "xcopy /s /y \"" .. ext .. "\\SDL3\\x64\\%{cfg.buildcfg}\\*.*\" \"$(TargetDir)\""
        }

    filter "configurations:ReleaseWithSymbols"
        postbuildcommands {
            "xcopy /s /y \"" .. ext .. "\\SDL3\\x64\\Release\\*.*\" \"$(TargetDir)\""
        }

    filter {}