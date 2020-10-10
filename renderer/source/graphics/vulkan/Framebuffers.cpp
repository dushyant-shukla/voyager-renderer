#include "Framebuffers.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

#include <array>

namespace vr
{
	Framebuffers::Framebuffers()
	{
	}

	Framebuffers::~Framebuffers()
	{
		if (!mFramebuffers.empty())
		{
			for (auto framebuffer : mFramebuffers)
			{
				vkDestroyFramebuffer(LOGICAL_DEVICE, framebuffer, ALLOCATION_CALLBACK);
			}

			RENDERER_DEBUG("RESOURCE DESTROYED: FRAME BUFFERS");
		}
	}

	VkFramebuffer& Framebuffers::operator[](const unsigned int& index)
	{
		return mFramebuffers[index];
	}

	void Framebuffers::Create(const std::vector<SwapchainImage> swapchainImages, const VkExtent2D& swapchainExtent, const VkImageView& depthbufferImageView, const VkRenderPass& renderPass)
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
			framebufferCreateInfo.width = swapchainExtent.width;
			framebufferCreateInfo.height = swapchainExtent.height;
			framebufferCreateInfo.layers = 1;

			CHECK_RESULT(vkCreateFramebuffer(LOGICAL_DEVICE, &framebufferCreateInfo, ALLOCATION_CALLBACK, &mFramebuffers[i]), "RESOURCE CREATION FAILED: FRAME BUFFERS");
		}

		RENDERER_DEBUG("RESOURCE CREATED: FRAME BUFFERS");
	}
}