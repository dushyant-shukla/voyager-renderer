------------------------------------------------------------- RENDERER LIBRARY PROJECT CONFIGURATION ------------------------------------------------------

project "renderer"
  kind      "StaticLib"
  language  "C++"
  characterset ("MBCS")

  targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
  objdir ("../bin-intermediate/" .. outputdir .. "/%{prj.name}")

  files
  {
    "main.h",
    "source/**.h",
    "source/**.cpp"
  }

  includedirs
  {
    "source",
    "../%{include_dir.glfw}",
    "../%{include_dir.stbi}",
    "../%{include_dir.glm}",
    "../%{include_dir.jsoncpp}",
    "../%{include_dir.spdlog}",
    "../%{include_dir.imgui}",
    "../%{include_dir.vulkan}",
	  "../%{include_dir.tiny_gltf}",
	  "../%{include_dir.jsonhpp}",
	  "../%{include_dir.assimp}"
  }

  libdirs
  {
	  "../%{lib_dir.vulkan}"
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
		"../%{lib_dir.assimp_debug}"
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
		"../%{lib_dir.assimp_release}"
	}
	links
	{
		"%{lib_name.assimp_release}"
	}
