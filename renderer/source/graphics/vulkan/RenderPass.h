#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class RenderPass
	{
	public:

		RenderPass();
		~RenderPass();

		void SetupDefaultRenderPass(const VkSurfaceFormatKHR& surfaceFormat, const VkFormat& depthBufferFormat);

		const VkRenderPass& GetVulkanRenderPass();

	private:

		VkSurfaceFormatKHR mSwapchainFormat;

		VkRenderPass mRenderPass = VK_NULL_HANDLE;
	};
}
