#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class Texture
	{
	public:

		Texture();
		~Texture();

		void Create(const char* filename, VkCommandPool transferCommandPool, VkQueue transferQueue);

		const VkImageView& GetVulkanImageView();
		const VkImage& GetVulkanImage();

	private:

		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
	};
}
