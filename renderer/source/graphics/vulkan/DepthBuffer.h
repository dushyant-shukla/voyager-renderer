#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class DepthBuffer
	{
	public:

		DepthBuffer(const VkPhysicalDevice& physicalDevice, const VkDevice& device, VkAllocationCallbacks* allocationCallbacks);
		~DepthBuffer();

		void CreateDefault(const VkRect2D& swapchainExtent);

		const VkImageView& GetImageView();

	private:

		void ChooseSupportedFormat();

	private:

		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		VkFormat mFormat;
		VkImage mImage;
		VkImageView mImageView;
		VkDeviceMemory mMemory;
	};
}
