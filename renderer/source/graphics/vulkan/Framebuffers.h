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

		void Create(const std::vector<SwapchainImage> swapchainImages, const VkExtent2D& swapchainExtent, const VkImageView& depthbufferImageView, const VkRenderPass& renderPass);
		VkFramebuffer& operator[](const unsigned int& index);

	private:

	private:

		std::vector<VkFramebuffer> mFramebuffers;
	};
}
