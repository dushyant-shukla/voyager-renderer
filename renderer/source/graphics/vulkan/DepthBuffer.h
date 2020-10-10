#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class DepthBuffer
	{
	public:

		DepthBuffer();
		~DepthBuffer();

		void CreateDefault(const VkExtent2D& swapchainExtent);

		const VkImageView& GetImageView();
		const VkFormat& GetFormat();

	private:

		void ChooseSupportedFormat();

	private:

		VkFormat mFormat;
		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
	};
}
