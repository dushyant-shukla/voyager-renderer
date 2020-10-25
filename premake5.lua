solution "voyager-renderer"
  architecture "x86_64"
  startproject "vulkan-basics"

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
include_dir["tiny_gltf"]= "externals/tiny_gltf"
include_dir["jsonhpp"]  = "externals/jsonhpp"
include_dir["assimp"]   = "externals/assimp/include"

lib_name = {}
lib_name["vulkan"]  = "vulkan-1.lib"
lib_name["opengl"]  = "opengl32.lib"
lib_name["assimp_release"] = "assimp-vc142-mt.lib"
lib_name["assimp_debug"]   = "assimp-vc142-mtd.lib"

lib_dir = {}
lib_dir["vulkan"] = "externals/vulkan/"
lib_dir["assimp_release"] = "externals/assimp/release"
lib_dir["assimp_debug"]   = "externals/assimp/debug"

dll_name = {}

-- build non-header-only library
-- these projects have premake script
include "externals/glfw"
include "externals/imgui"
include "externals/jsoncpp"

--include "scenes/animation-motion-along-path"

------------------------------------------------------------- RENDERER LIBRARY PROJECT CONFIGURATION ------------------------------------------------------

project "renderer"
  location  "renderer"
  kind      "StaticLib"
  language  "C++"
  characterset ("MBCS")

  targetdir ("bin/" .. outputdir .. "/%{prj.name}")
  objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")

  files
  {
	"%{prj.name}/main.h",
    "%{prj.name}/source/**.h",
    "%{prj.name}/source/**.cpp"
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
    "%{include_dir.vulkan}",
	"%{include_dir.tiny_gltf}",
	"%{include_dir.jsonhpp}",
	"%{include_dir.assimp}"
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
	defines
	{
		"ENABLE_VALIDATION",
		"ENABLE_DEBUG_LOGGING"
	}
	libdirs
	{
		"%{lib_dir.assimp_debug}"
	}
	links
	{
		"%{lib_name.assimp_debug}"
	}
  

  filter "configurations:Release"
	buildoptions "/MD"
	optimize "On"
	defines
	{
	}
	libdirs
	{
		"%{lib_dir.assimp_release}"
	}
	links
	{
		"%{lib_name.assimp_release}"
	}

------------------------------------------------------------- PROJECT MODEL-LOADING CONFIGURATION ------------------------------------------------------

project "model-loading"
  location  "scenes/model-loading"
  kind      "ConsoleApp"
  language  "C++"
  characterset ("MBCS")

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
	"%{include_dir.spdlog}",
	"%{include_dir.vulkan}",
	"%{include_dir.assimp}"
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

------------------------------------------------------------- PROJECT VULKAN-BASICS CONFIGURATION ------------------------------------------------------

project "vulkan-basics"
  location  "scenes/vulkan-basics"
  kind      "ConsoleApp"
  language  "C++"
  characterset ("MBCS")

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
	"%{include_dir.vulkan}",
	"%{include_dir.spdlog}",
	"%{include_dir.stbi}",
	"%{include_dir.assimp}"
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
	
	
-------------------------------------------------------------- PROJECT ANIMATION KEYFRAMES CONFIGURATION ------------------------------------------------------

project "animation-keyframes"
  location  "scenes/animation-keyframes"
  kind      "ConsoleApp"
  language  "C++"
  characterset ("MBCS")

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
	"%{include_dir.vulkan}",
	"%{include_dir.spdlog}",
	"%{include_dir.stbi}",
	"%{include_dir.assimp}"
  }

  links
  {
    "renderer"
  }
  
  postbuildcommands
  {
	("{COPY} $(SolutionDir)assets $(SolutionDir)bin/assets")
  }

  filter "system:windows"
	cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"
	
  filter "configurations:Debug"
	buildoptions "/MDd"
	symbols "On"
	postbuildcommands
	{
		("{COPY} $(SolutionDir)externals\\assimp\\debug\\assimp-vc142-mtd.dll $(SolutionDir)scenes\\%{prj.name}\\")
	}
	
  filter "configurations:Release"
	buildoptions "/MD"
	optimize "On"
	postbuildcommands
	{
		("{COPY} $(SolutionDir)externals\\assimp\\release\\assimp-vc142-mt.dll $(SolutionDir)scenes\\%{prj.name}\\")
	}
	
-------------------------------------------------------------- PROJECT ANIMATION MOTION ALONG A PATH CONFIGURATION ------------------------------------------------------

project "animation-motion-along-path"
  location  "scenes/animation-motion-along-path"
  kind      "ConsoleApp"
  language  "C++"
  characterset ("MBCS")

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
	"%{include_dir.vulkan}",
	"%{include_dir.spdlog}",
	"%{include_dir.stbi}",
	"%{include_dir.assimp}"
  }

  links
  {
    "renderer"
  }
  
  postbuildcommands
  {
	("{COPY} $(SolutionDir)assets $(SolutionDir)bin/assets")
  }

  filter "system:windows"
	cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"
	
  filter "configurations:Debug"
	buildoptions "/MDd"
	symbols "On"
	postbuildcommands
	{
		("{COPY} $(SolutionDir)externals\\assimp\\debug\\assimp-vc142-mtd.dll $(SolutionDir)scenes\\%{prj.name}\\")
	}
	
  filter "configurations:Release"
	buildoptions "/MD"
	optimize "On"
	postbuildcommands
	{
		("{COPY} $(SolutionDir)externals\\assimp\\release\\assimp-vc142-mt.dll $(SolutionDir)scenes\\%{prj.name}\\")
	}