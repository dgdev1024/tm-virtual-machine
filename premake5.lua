-- @file    premake5.lua
-- @brief   Premake5 build script for tm-virtual-machine.

-- Workspace Settings
workspace "tm-virtual-machine"

    -- Language and Standard
    language "C"
    cdialect "C17"

    -- Build File Location
    location "./generated"

    -- Build Configuration
    configurations { "debug", "release", "distribute" }
    filter { "configurations:debug" }
        defines { "TM_DEBUG" }
        symbols "On"
    filter { "configurations:release" }
        defines { "TM_RELEASE" }
        optimize "On"
    filter { "configurations:distribute" }
        defines { "TM_DISTRIBUTE" }
        optimize "On"
    filter { "system:linux" }
        defines { "TM_LINUX" }
        cdialect "gnu17"
    filter {}

    -- "tm" - TM Virtual Machine
    project "tm"

        -- Shared Library
        kind "SharedLib"

        -- Project Location
        location "./generated/tm"
        targetdir "./build/bin/tm/%{cfg.buildcfg}"
        objdir "./build/obj/tm/%{cfg.buildcfg}"

        -- Project Files
        includedirs {
            "./projects/tm/include"
        }
        files {
            "./projects/tm/src/**.c"
        }

    -- "tmm" - TM Virtual Machine Assembler
    project "tmm"

        -- Console Application
        kind "ConsoleApp"

        -- Project Location
        location "./generated/tmm"
        targetdir "./build/bin/tmm/%{cfg.buildcfg}"
        objdir "./build/obj/tmm/%{cfg.buildcfg}"

        -- Project Files
        includedirs {
            "./projects/tm/include",
            "./projects/tmm/include"
        }
        files {
            "./projects/tmm/src/**.c"
        }

        -- Library Dependencies
        libdirs {
            "./build/bin/tm/%{cfg.buildcfg}"
        }
        links {
            "tm", "m"
        }
        
    -- "tomboy" - Gameboy-like Emulation Backend Powered by TM
    project "tomboy"

        -- Console Application
        kind "SharedLib"

        -- Project Location
        location "./generated/tomboy"
        targetdir "./build/bin/tomboy/%{cfg.buildcfg}"
        objdir "./build/obj/tomboy/%{cfg.buildcfg}"

        -- Project Files
        includedirs {
            "./projects/tm/include",
            "./projects/tomboy/include"
        }
        files {
            "./projects/tomboy/src/**.c"
        }

    -- "tomboy-sdl2" - SDL2 Frontend for TOMBOY
    project "tomboy-sdl2"

        -- Console Application
        kind "ConsoleApp"

        -- Project Location
        location "./generated/tomboy-sdl2"
        targetdir "./build/bin/tomboy-sdl2/%{cfg.buildcfg}"
        objdir "./build/obj/tomboy-sdl2/%{cfg.buildcfg}"

        -- Project Files
        includedirs {
            "./projects/tm/include",
            "./projects/tomboy/include",
            "./projects/tomboy-sdl2/include"
        }
        files {
            "./projects/tomboy-sdl2/src/**.c"
        }

        -- Library Dependencies
        libdirs {
            "./build/bin/tm/%{cfg.buildcfg}",
            "./build/bin/tomboy/%{cfg.buildcfg}"
        }
        links {
            "tomboy", "tm", "SDL2", "m"
        }
