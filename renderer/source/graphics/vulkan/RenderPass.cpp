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

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// compute subpasses are also possible
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
}