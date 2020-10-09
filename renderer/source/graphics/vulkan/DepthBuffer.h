#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class DepthBuffer
	{
	public:

		DepthBuffer();
		~DepthBuffer();

		void CreateDefault(const VkPhysicalDevice& physicalDevice, const VkDevice& device, VkAllocationCallbacks* allocationCallbacks, const VkExtent2D& swapchainExtent);

		const VkImageView& GetImageView();
		const VkFormat& GetFormat();

	private:

		void ChooseSupportedFormat();

	private:

		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		VkFormat mFormat;
		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
	};
}
