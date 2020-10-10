#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class TextureSampler
	{
	public:

		TextureSampler();
		~TextureSampler();

		void CreateDefault();

		const VkSampler& GetVulkanSampler();

	private:

		VkSamplerCreateInfo mSamplerInfo;

		VkSampler mSampler = VK_NULL_HANDLE;
	};
}
