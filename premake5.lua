
local PROJECT_NAME = "EnergySaverDeluxe"

-- premake5.lua
workspace (PROJECT_NAME)
   configurations { "Debug", "Release" }

project (PROJECT_NAME)
   kind "ConsoleApp"
   language "C"
   cdialect "C99"
   targetdir "bin/%{cfg.buildcfg}"

   buildoptions { "-Wall", "-Wextra", "-Werror", "-Wpedantic" }
   links { "pthread", "curl" }

   includedirs { "include/core/" }

   files { "**.h", "**.c" }
   removefiles { "include/core/tests/**" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

newaction {
    trigger     = "run",
    description = "Build and run the project on Ubuntu",
    execute = function ()
        os.execute("premake5 gmake")
        os.execute("make")
        os.execute("./bin/Debug/" .. PROJECT_NAME)
    end
}

newaction {
    trigger     = "clean",
    description = "Clean the build folders/files on Ubuntu",
    execute = function ()
        os.execute("rm -r bin")
        os.execute("rm -r obj")
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