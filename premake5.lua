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
if _OPTIONS["q2path"] then
    local lastchar = string.sub(_OPTIONS["q2path"], #_OPTIONS["q2path"])
    if lastchar ~= "/" and lastchar ~= "\\" then
        _OPTIONS["q2path"] = _OPTIONS["q2path"] .. "/"
    end
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
    targetname "gamex86"
    architecture "x86"

    debugcommand (_OPTIONS["q2path"] .. _OPTIONS["q2exe"])
    debugargs { _OPTIONS["q2args"] }

    files { "jumpserver/**.h", "jumpserver/**.c", "jumpserver/**.cpp" }

    includedirs { "packages/tencent.rapidjson.1.1.1/lib/native/include" }

    filter { "files:**.c" }
        compileas "C++"

    -- SQLite has to be compiled as C.
    filter { "files:jumpserver/sqlite3.c" }
        compileas "C"
    filter { "files:jumpserver/shell.c" }
        compileas "C"

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "system:Windows"
        defines { "WIN32" }

    -- Warnings
    filter "system:linux"
        disablewarnings "write-strings"
    filter "system:Windows"
        disablewarnings "4996"

    -- Post-build commands
    filter "system:Windows"
        -- Move game library + debug symbols to game folder.
        postbuildcommands {
            "copy /B /Y \"$(OutDir)$(TargetFileName)\" \"" .. _OPTIONS["q2path"] .. _OPTIONS["q2jumpdir"] .. "\"",
            "copy /B /Y \"$(OutDir)$(TargetName).pdb\" \"" .. _OPTIONS["q2path"] .. _OPTIONS["q2jumpdir"] .. "\""
        }

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
