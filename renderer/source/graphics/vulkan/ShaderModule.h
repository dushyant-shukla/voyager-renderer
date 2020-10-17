#pragma once

#include <vulkan/vulkan.h>
#include <string>

namespace vr
{
	inline const std::string SHADER_BASE_PATH = "..\\..\\assets\\shaders\\";

	class ShaderModule
	{
	public:

		ShaderModule(std::string filename);
		~ShaderModule();

		const VkShaderModule GetShaderModule();

		std::string& GetFilename();

	private:

		VkShaderModule mModule;

		std::string mFilename;
	};
}
