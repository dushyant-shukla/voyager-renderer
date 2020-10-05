#pragma once

#include <vulkan/vulkan.h>
#include <string>

namespace vr
{
	class ShaderModule
	{
	public:

		ShaderModule(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks, std::string filename);
		~ShaderModule();

		const VkShaderModule GetShaderModule();

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;
		VkShaderModule mModule;
	};
}
