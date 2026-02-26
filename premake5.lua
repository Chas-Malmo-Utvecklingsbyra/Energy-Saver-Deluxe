
local PROJECT_NAME = "EnergySaverDeluxe"
local BUILD_DIR = "build/"

-- premake5.lua
workspace (PROJECT_NAME)
   configurations { "Debug", "Release" }

project (PROJECT_NAME)
   kind "ConsoleApp"
   language "C"
   cdialect "C99"

   targetdir (BUILD_DIR .. "bin/%{cfg.buildcfg}")
   objdir (BUILD_DIR .. "obj/%{cfg.buildcfg}")

   buildoptions { "-Wall", "-Wextra", "-Werror", "-Wpedantic" }
   links { "pthread", "curl" }

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
      removefiles { "**.h", "**.c" }
      files { "client/src/**.c", "client/src/**.h" }

newaction {
    trigger     = "server",
    description = "Build and run the server on Ubuntu",
    execute = function ()
        os.execute("premake5 clean")
        os.execute("premake5 gmake")
        os.execute("make")
        os.execute("./" .. BUILD_DIR .. "bin/Debug/" .. PROJECT_NAME)
    end
}

newaction {
    trigger     = "client",
    description = "Build and run the client on Ubuntu",
    execute = function ()
        os.execute("premake5 clean")
        os.execute("premake5 gmake --type=client")
        os.execute("make")
        os.execute("./" .. BUILD_DIR .. "bin/Debug/" .. PROJECT_NAME)
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
        os.execute("valgrind --leak-check=yes ./" .. BUILD_DIR .. "bin/Debug/" .. PROJECT_NAME)
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