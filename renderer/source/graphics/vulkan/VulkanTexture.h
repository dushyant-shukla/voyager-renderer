#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class VulkanTexture
	{
	public:

		VulkanTexture();
		VulkanTexture(unsigned int binding);
		~VulkanTexture();

		/*
			Must update descriptor set while loading the texture
		*/
		void LoadFromFile(const char* filename);
		void LoadFromFile(const char* filename, const VkSampler& sampler);
		void LoadWithData(const unsigned char* data, const VkDeviceSize size, const int width, const int height, const VkSampler& sampler);

		const VkImageView& GetVulkanImageView();
		const VkImage& GetVulkanImage();

	public:

		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkDescriptorImageInfo mImageInfo = {};
		unsigned int mBinding;
	};
}
