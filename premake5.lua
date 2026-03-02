
local PROJECT_NAME = "EnergySaverDeluxe"
local BUILD_DIR = "build/"
local CLIENT_OR_SERVER = "/server/"

-- premake5.lua
workspace (PROJECT_NAME)
   configurations { "Debug", "Release" }

project (PROJECT_NAME)
   kind "ConsoleApp"
   language "C"
   cdialect "C99"

   targetdir (BUILD_DIR .. "bin/%{cfg.buildcfg}/server")
   objdir (BUILD_DIR .. "obj/%{cfg.buildcfg}/server")

   buildoptions { "-Wall", "-Wextra", "-Werror", "-Wpedantic" }
   links { "pthread", "curl", "m" }

   includedirs { "include/core/" }

   files { "**.h", "**.c" }
   removefiles { "include/core/tests/**" }
   removefiles { "client/**" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

    filter "options:type=client"
      language "C++"
      cppdialect "C++17"
      targetdir (BUILD_DIR .. "bin/%{cfg.buildcfg}/client")
      objdir (BUILD_DIR .. "obj/%{cfg.buildcfg}")
      removefiles { "**.h", "**.c", "server/**" }
      files { "client/src/**.cpp", "client/src/**.h" }

newaction {
    trigger     = "server",
    description = "Build and run the server on Ubuntu",
    execute = function ()
        os.execute("premake5 gmake")
        os.execute("make")
        os.execute("./" .. BUILD_DIR .. "bin/Debug/server/" .. PROJECT_NAME)
    end
}

newaction {
    trigger     = "client",
    description = "Build and run the client on Ubuntu",
    execute = function ()
        CLIENT_OR_SERVER = "/client/"
        os.execute("premake5 gmake --type=client")
        os.execute("make")
        os.execute("./" .. BUILD_DIR .. "bin/Debug/client/" .. PROJECT_NAME)
    end
}

newaction {
    trigger     = "clean",
    description = "Clean the build folders/files on Ubuntu",
    execute = function ()
        os.execute("rm -r build")
        os.execute("rm " .. PROJECT_NAME .. ".make")
        os.execute("rm Makefile")
    end
}

newaction {
    trigger     = "build",
    description = "Build the project on Ubuntu",
    execute = function ()
        os.execute("premake5 gmake")
        os.execute("make")
    end
}

newaction {
    trigger     = "valgrind",
    description = "Use valgrind on Ubuntu",
    execute = function ()
        os.execute("premake5 gmake")
        os.execute("make")
        os.execute("valgrind --leak-check=yes ./" .. BUILD_DIR .. "bin/Debug/" .. CLIENT_OR_SERVER .. PROJECT_NAME)
    end
}

newoption {
    trigger = "type",
    value = "whatever",
    description = "Choose server or client",
    allowed = {
        { "client", "Client" },
        { "server", "Server" }
    }
}

-- valgrind --leak-check=yes $(BIN)