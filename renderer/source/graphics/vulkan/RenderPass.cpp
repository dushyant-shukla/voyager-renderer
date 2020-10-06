#include "RenderPass.h"

#include "utility/RendererCoreUtility.h"

vr::RenderPass::RenderPass(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks, const VkSurfaceFormatKHR& format)
	: mLogicalDevice(device), mAllocationCallbacks(allocationCallbacks), mSwapchainFormat(format)
{
}

vr::RenderPass::~RenderPass()
{
	vkDestroyRenderPass(mLogicalDevice, mRenderPass, mAllocationCallbacks);
	RENDERER_DEBUG("RESOURCE DESTROYED: RENDER PASS");
}

void vr::RenderPass::SetupDefaultRenderPass(const VkFormat& depthBufferFormat)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = mSwapchainFormat.format;					// image format to use for attachment
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// number of samples to write for multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// describes what to do with attachment before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;				// describes what to do with the attachment after rendering
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// describes what to do with the stencil buffer before rendering
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// describes what to do with the stencil buffer after rendering

	// Framebuffer data will be stores as an image, but image can be given different data layouts to give optimal use for certain operations
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// image data layout before renderpass starts
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// image data layout after renderpass (to change to),  ready for presentation using the swap chain after rendering

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// depth attachments
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depthBufferFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;									// position 1 in the array of attachments
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// compute subpasses are also possible
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	// Need to determine when layout transition occurs using subpass dependencies
	std::array<VkSubpassDependency, 2> subpassDependencies;
	// Need to convert from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;																// Subpass index (VK_SUBPASS_EXTERNAL = special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;												// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;														// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// Need to convert from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[1].srcSubpass = 0;														// Subpass index (VK_SUBPASS_EXTERNAL = special value meaning outside of renderpass)
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		// Pipeline stage
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> renderPassAttachments = { colorAttachment, depthAttachment };
	// create info for render pass
	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = static_cast<unsigned int>(renderPassAttachments.size());
	renderpassCreateInfo.pAttachments = renderPassAttachments.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = static_cast<unsigned int> (subpassDependencies.size());
	renderpassCreateInfo.pDependencies = subpassDependencies.data();

	CHECK_RESULT(vkCreateRenderPass(mLogicalDevice, &renderpassCreateInfo, nullptr, &mRenderPass), "RESOURCE CREATION FAILED: RENDER PASS");
	RENDERER_DEBUG("RESOURCE CREATED: RENDER PASS");
}