------------------------------------------------------------- PROJECT MODEL-LOADING CONFIGURATION ------------------------------------------------------

project "model-loading"
  kind      "ConsoleApp"
  language  "C++"
  characterset ("MBCS")

  targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
  objdir ("../../bin-intermediate/" .. outputdir .. "/%{prj.name}")

  files
  {
		"source/**.h",
		"source/**.cpp"
  }

  includedirs
  {
	  "../../renderer",
	  "../../renderer/source",
    "../../%{include_dir.glm}",
    "../../%{include_dir.imgui}",
    "../../%{include_dir.jsoncpp}",
	  "../../%{include_dir.spdlog}",
	  "../../%{include_dir.vulkan}",
	  "../../%{include_dir.assimp}"
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

  filter "configurations:Release"
	buildoptions "/MD"
	optimize "On"