#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "Swapchain.h"

namespace vr
{
	class Framebuffers
	{
	public:

		Framebuffers(const VkDevice& logicalDevice, VkAllocationCallbacks* mAllocationCallbacks);
		~Framebuffers();

		void Create(const std::vector<SwapchainImage> swapchainImages, const VkRect2D& swapchainExtent, const VkImageView& depthbufferImageView, const VkRenderPass& renderPass);

	private:

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::vector<VkFramebuffer> mFramebuffers;
	};
}
