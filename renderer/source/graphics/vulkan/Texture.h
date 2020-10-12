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
		void LoadFromFile(const char* filename, VkCommandPool transferCommandPool, VkQueue transferQueue);

		const VkImageView& GetVulkanImageView();
		const VkImage& GetVulkanImage();

	private:

		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
	};
}
