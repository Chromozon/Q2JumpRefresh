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
