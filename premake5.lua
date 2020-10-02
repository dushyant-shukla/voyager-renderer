solution "voyager-renderer"
  architecture  "x86_64"

	configurations {
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include_dir = {}
include_dir["glfw"]		= "externals/glfw/include"
include_dir["stbi"]		= "externals/stbi"
include_dir["glm"]		= "externals/glm"
include_dir["jsoncpp"] 	= "externals/jsoncpp/include"
include_dir["spdlog"] 	= "externals/spdlog/include"
include_dir["imgui"] 	= "externals/imgui"
include_dir["vulkan"]	= "externals/vulkan/include"

lib_name = {}
lib_name["vulkan"]  = "vulkan-1.lib"
lib_name["opengl"]  = "opengl32.lib"

lib_dir = {}
lib_dir["vulkan"] = "externals/vulkan/"

dll_name = {}

-- build non-header-only library
-- these projects have premake script
include "externals/glfw"
include "externals/imgui"
include "externals/jsoncpp"

project "renderer"
  location  "renderer"
  kind      "StaticLib"
  language  "C++"

  targetdir ("bin/" .. outputdir .. "/%{prj.name}")
  objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")

  files
  {
	"%{prj.name}/main.h",
    "%{prj.name}/source/**.h",
    "%{prj.name}/source/**.cpp",

    "%{prj.name}/externals/stbi/**.h",
    "%{prj.name}/externals/stbi/**.cpp",

    "%{prj.name}/externals/glm/glm/**.hpp",
    "%{prj.name}/externals/glm/glm/**.inl",

    "%{prj.name}/externals/jsoncpp/include/json/**.h",
    "%{prj.name}/externals/jsoncpp/src/lib_json/**.h",
    "%{prj.name}/externals/jsoncpp/src/lib_json/**.cpp",
    "%{prj.name}/externals/jsoncpp/src/lib_json/**.inl"
  }

  includedirs
  {
    "%{prj.name}/source",
    "%{include_dir.glfw}",
    "%{include_dir.stbi}",
    "%{include_dir.glm}",
    "%{include_dir.jsoncpp}",
    "%{include_dir.spdlog}",
    "%{include_dir.imgui}",
    "%{include_dir.vulkan}"
  }

  libdirs
  {
	"%{lib_dir.vulkan}"
  }

  links
  {
	"%{lib_name.vulkan}",
    "glfw",
    "imgui",
	"jsoncpp"
  }

  filter { "files:**.c" }
    compileas "C++"

  filter "system:windows"
    cppdialect "C++17"
    staticruntime "On"
    systemversion "latest"

  defines
  {
    "GLFW_INCLUDE_VULKAN"
  }

  postbuildcommands
  {
  }

  filter "configurations:Debug"
  buildoptions "/MDd"
  symbols "On"

  filter "configurations:Release"
  buildoptions "/MD"
  optimize "On"


project "animation-keyframes"
  location  "scenes/animation-keyframes"
  kind      "ConsoleApp"
  language  "C++"

  targetdir ("bin/" .. outputdir .. "/%{prj.name}")
  objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")

  files
  {
		"scenes/%{prj.name}/source/**.h",
		"scenes/%{prj.name}/source/**.cpp"
  }

  includedirs
  {
	"renderer",
	"renderer/source",
    "%{include_dir.glm}",
    "%{include_dir.imgui}",
    "%{include_dir.jsoncpp}",
  }

  links
  {
    "renderer"
  }
   
  filter "system:windows"
	cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"
	
  filter "configurations:Debug"
	buildoptions "/MDd"
	symbols "On"
	
  filter "configurations:Release"
	buildoptions "/MD"
	optimize "On"


  
