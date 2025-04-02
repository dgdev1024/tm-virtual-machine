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
        