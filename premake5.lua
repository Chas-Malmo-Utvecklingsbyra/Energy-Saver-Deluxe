
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
      buildoptions { "-Wall", "-Wextra", "-Werror", "-Wpedantic" }
      files { "**.c", "**.cpp", "**.h" }
      removefiles { "server/**", "include/core/tests/**" }

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


local function setup_config()
    local settings_template = [[
    {
    "http_server_port": 8080,
    "exec_fetcher_on_startup": true,
    "run_as_daemon": false,
    "fetcher_exec_path": "/bin/http-request-service",
    "fetchers_commands_count": 2,
    "fetchers_commands_args": [
        "-i 60 -u 'https://api.open-meteo.com' -r '/v1/forecast?latitude=55.71&longitude=13.19&minutely_15=direct_radiation,diffuse_radiation,direct_normal_irradiance,temperature_2m,weather_code' -o %s/data/weather -n weather_SE4.json",
        "-i 60 -u 'https://www.elprisetjustnu.se' -r '/api/v1/prices/' -o %s/data/price -n price.json"
        ]
    }
    ]]

    local function getcwd()
        local handle = io.popen("pwd")
        local result = handle:read("*a")
        handle:close()
        return result:gsub("\n", "")
    end

    local file = io.open("settings.json", "w")
    local settings = string.format(settings_template, getcwd(), getcwd())
    file:write(settings)
    file:close()
end

newaction {
    trigger     = "install",
    description = "Installs http-request-service and sets up the config file",
    execute = function ()
        os.execute("git clone --recurse-submodules https://github.com/Chas-Malmo-Utvecklingsbyra/http-request-service.git")
        os.execute("cd http-request-service && premake5 build")
        os.execute("sudo mv ./http-request-service/build/bin/Debug/http-request-service /bin/ && cd ..")
        os.execute("rm -rf http-request-service")

        setup_config()

        print("Succesfully installed the dependencies!")
    end
}

newaction {
    trigger     = "uninstall",
    description = "Uninstall http-request-service",
    execute = function ()
        os.execute("sudo rm /bin/http-request-service")

        print("Succesfully uninstalled the program!")
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