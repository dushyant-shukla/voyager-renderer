#include "Framebuffers.h"
#include "utility/RendererCoreUtility.h"
#include <array>

namespace vr
{
	Framebuffers::Framebuffers(const VkDevice& logicalDevice, VkAllocationCallbacks* allocationCallbacks)
		: mLogicalDevice(logicalDevice), mAllocationCallbacks(allocationCallbacks)
	{
	}

	Framebuffers::~Framebuffers()
	{
		for (auto framebuffer : mFramebuffers)
		{
			vkDestroyFramebuffer(mLogicalDevice, framebuffer, mAllocationCallbacks);
		}

		RENDERER_DEBUG("RESOURCE DESTROYED: FRAME BUFFERS");
	}

	void Framebuffers::Create(const std::vector<SwapchainImage> swapchainImages, const VkRect2D& swapchainExtent, const VkImageView& depthbufferImageView, const VkRenderPass& renderPass)
	{
		mFramebuffers.resize(swapchainImages.size());

		for (size_t i = 0; i < swapchainImages.size(); ++i)
		{
			std::array<VkImageView, 2> attachments = {
				swapchainImages[i].imageView,
				depthbufferImageView
			};

			VkFramebufferCreateInfo framebufferCreateInfo = {};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = renderPass;
			framebufferCreateInfo.attachmentCount = static_cast<unsigned int> (attachments.size());
			framebufferCreateInfo.pAttachments = attachments.data();
			framebufferCreateInfo.width = swapchainExtent.extent.width;
			framebufferCreateInfo.height = swapchainExtent.extent.height;
			framebufferCreateInfo.layers = 1;

			CHECK_RESULT(vkCreateFramebuffer(mLogicalDevice, &framebufferCreateInfo, nullptr, &mFramebuffers[i]), "RESOURCE CREATION FAILED: FRAME BUFFERS");
			RENDERER_DEBUG("RESOURCE CREATED: FRAME BUFFERS");
		}
	}
}