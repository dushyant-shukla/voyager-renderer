#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class Texture
	{
	public:

		Texture();
		~Texture();

		/*
			Must update descriptor set while loading the texture
		*/
		void LoadFromFile(const char* filename);
		void LoadFromFile(const char* filename, const VkSampler& sampler);

		const VkImageView& GetVulkanImageView();
		const VkImage& GetVulkanImage();

	public:

		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkDescriptorImageInfo mImageInfo = {};
	};
}
