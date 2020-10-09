#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class TextureSampler
	{
	public:

		TextureSampler();
		~TextureSampler();

		void CreateDefault(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks);

		TextureSampler& Initialize(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks);

		const VkSampler& GetVulkanSampler();

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		VkSamplerCreateInfo mSamplerInfo;

		VkSampler mSampler = VK_NULL_HANDLE;
	};
}
