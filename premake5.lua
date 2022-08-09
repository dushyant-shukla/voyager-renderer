solution "voyager-renderer"
  architecture "x86_64"
  startproject "vulkan-basics"

	configurations {
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include_dir = {}
include_dir["glfw"]		  	= "externals/glfw/include"
include_dir["stbi"]		  	= "externals/stbi"
include_dir["glm"]		  	= "externals/glm"
include_dir["jsoncpp"] 		= "externals/jsoncpp/include"
include_dir["spdlog"] 		= "externals/spdlog/include"
include_dir["imgui"] 	  	= "externals/imgui"
include_dir["vulkan"]	  	= "externals/vulkan/include"
include_dir["tiny_gltf"]	= "externals/tiny_gltf"
include_dir["jsonhpp"]  	= "externals/jsonhpp"
include_dir["assimp"]   	= "externals/assimp/include"

lib_name = {}
lib_name["vulkan"]         = "vulkan-1.lib"
lib_name["opengl"]         = "opengl32.lib"
lib_name["assimp_release"] = "assimp-vc142-mt.lib"
lib_name["assimp_debug"]   = "assimp-vc142-mtd.lib"

lib_dir = {}
lib_dir["vulkan"]         = "externals/vulkan/"
lib_dir["assimp_release"] = "externals/assimp/release"
lib_dir["assimp_debug"]   = "externals/assimp/debug"

dll_name = {}

-- build non-header-only library
-- these projects have premake script
group "dependencies"
include "externals/glfw"
include "externals/imgui"
include "externals/jsoncpp"
group ""

group "core"
include "renderer"
group ""

include "scenes/vulkan-basics"
include "scenes/model-loading"
include "scenes/animation-motion-along-path"
include "scenes/cloth-simulation"
include "scenes/gpu-computing"
include "scenes/animation-keyframes"
include "scenes/animation-inverse-kinematics"
