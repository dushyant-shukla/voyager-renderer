#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class RenderPass
	{
	public:

		RenderPass(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks, const VkSurfaceFormatKHR& format);
		~RenderPass();

		void SetupDefaultRenderPass(const VkFormat& depthBufferFormat);

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;
		VkSurfaceFormatKHR mSwapchainFormat;

		VkRenderPass mRenderPass;
	};
}
