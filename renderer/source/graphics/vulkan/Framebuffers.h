#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "Swapchain.h"

namespace vr
{
	class Framebuffers
	{
	public:

		Framebuffers();
		~Framebuffers();

		void Create(const VkDevice& logicalDevice, VkAllocationCallbacks* allocationCallbacks, const std::vector<SwapchainImage> swapchainImages, const VkExtent2D& swapchainExtent, const VkImageView& depthbufferImageView, const VkRenderPass& renderPass);

	private:

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::vector<VkFramebuffer> mFramebuffers;
	};
}
