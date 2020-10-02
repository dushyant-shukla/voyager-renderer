project "jsoncpp"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/json/allocator.h",
		"include/json/assertions.h",
		"include/json/config.h",
		"include/json/forwards.h",
		"include/json/json.h",
		"include/json/json_features.h",
		"include/json/reader.h",
		"include/json/value.h",
		"include/json/version.h",
		"include/json/write.h",
		"src/lib_json/json_reader.cpp",
		"src/lib_json/json_tool.h",
		"src/lib_json/json_value.cpp",
		"src/lib_json/json_valueiterator.inl",
		"src/lib_json/json_writer.cpp"
	}

	includedirs
	{
		"include"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"