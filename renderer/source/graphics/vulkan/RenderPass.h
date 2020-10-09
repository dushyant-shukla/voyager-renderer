#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class RenderPass
	{
	public:

		RenderPass();
		~RenderPass();

		void SetupDefaultRenderPass(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks, const VkSurfaceFormatKHR& surfaceFormat, const VkFormat& depthBufferFormat);

		const VkRenderPass& GetVulkanRenderPass();

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;
		VkSurfaceFormatKHR mSwapchainFormat;

		VkRenderPass mRenderPass = VK_NULL_HANDLE;
	};
}
