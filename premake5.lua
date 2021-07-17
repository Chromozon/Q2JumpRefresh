DEFAULT_GAME_FOLDER = "jumprefresh"

--[[
Command line arguments
--]]
newoption {
    trigger = "q2path",
    description = "Path to Quake 2 client.",
    value = "C:\\Games\\Quake2\\",
    default = ""
}

newoption {
    trigger = "q2exe",
    description = "Quake 2 client executable name.",
    value = "q2pro.exe",
    default = "q2pro.exe"
}

newoption {
    trigger = "q2args",
    description = "Quake 2 client arguments.",
    value = "+game " .. DEFAULT_GAME_FOLDER,
    default = "+game " .. DEFAULT_GAME_FOLDER
}

newoption {
    trigger = "q2jumpdir",
    description = "Quake 2 jump folder name. Defaults to " .. DEFAULT_GAME_FOLDER,
    default = DEFAULT_GAME_FOLDER
}

-- Append slash to the end of Quake 2 path.
if _OPTIONS["q2path"] and #_OPTIONS["q2path"] > 0 then
    local lastchar = string.sub(_OPTIONS["q2path"], #_OPTIONS["q2path"])
    if lastchar ~= "/" and lastchar ~= "\\" then
        _OPTIONS["q2path"] = _OPTIONS["q2path"] .. "/"
    end
else
    _OPTIONS["q2path"] = nil
end

workspace "jump"
    configurations { "Debug", "Release" }

--[[
Jump Game Library
C++ Project
--]]
project "jumpserver"
    basedir "jumpserver"
    kind "SharedLib"
    language "C++"
    compileas "C++"
    cppdialect "C++17"
    targetprefix ""
    targetname "gamex86"
    architecture "x86"

    if _OPTIONS["q2path"] then
        debugcommand (_OPTIONS["q2path"] .. _OPTIONS["q2exe"])
        debugargs { _OPTIONS["q2args"] }
    end

    files { "jumpserver/**.h", "jumpserver/**.c", "jumpserver/**.cpp" }
    includedirs { "jumpserver/thirdparty" }

    filter "files:**.c"
        compileas "C++"

    -- SQLite has to be compiled as C.
    filter { "files:jumpserver/thirdparty/sqlite/*.c" }
        compileas "C"

    -- Specific targetnames
    filter { "system:linux", "architecture:x86" }
        targetname "gamei386"
    filter "architecture:x86_64"
        targetname "gamex86_64"

    -- Configuration specific
    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"
        
    -- System specific
    filter "system:linux"
        disablewarnings "write-strings"
        -- Move game library to game folder.
        if _OPTIONS["q2path"] then
            postbuildcommands {
                '{COPYFILE} "%{cfg.buildtarget.directory}/%{cfg.buildtarget.name}" "' .. _OPTIONS["q2path"] .. _OPTIONS["q2jumpdir"] .. '"'
            }
        end
    filter "system:Windows"
        defines { "WIN32" }
        disablewarnings "4996"
        -- Move game library + debug symbols to game folder.
        if _OPTIONS["q2path"] then
            postbuildcommands {
                '{COPYFILE} "%{cfg.buildtarget.directory}%{cfg.buildtarget.name}" "' .. _OPTIONS["q2path"] .. _OPTIONS["q2jumpdir"] .. '"',
                '{COPYFILE} "%{cfg.buildtarget.directory}%{cfg.buildtarget.basename}.pdb" "' .. _OPTIONS["q2path"] .. _OPTIONS["q2jumpdir"] .. '"'
            }
        end

--[[
Jump Database
C# Project
--]]
project "jumpdatabase"
    basedir "jumpdatabase"
    kind "ConsoleApp"
    language "C#"
    dotnetframework "netcoreapp3.1"

    nuget { "log4net:2.0.12", "Microsoft.Data.Sqlite:5.0.1", "Newtonsoft.Json:12.0.3" }

    files { "jumpdatabase/*.cs" }

--[[
Jump Database Test
C# Project
--]]
project "jumpdatabase_test"
    basedir "jumpdatabase_test"
    kind "ConsoleApp"
    language "C#"
    dotnetframework "netcoreapp3.1"

    nuget { "Newtonsoft.Json:12.0.3" }

    files { "jumpdatabase_test/*.cs" }
