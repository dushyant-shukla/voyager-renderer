#include "AnimationKeyFrames.h"
#include "graphics/vulkan/ShaderModule.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"
#include "graphics/vulkan/SynchronizationPrimitives.h"

#include <array>

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name),
		mPipeline(),
		mPipelineLayout()
	{}

	void AnimationKeyframes::InitializeScene()
	{
		SetupPipeline();
	}

	void AnimationKeyframes::SetupPipeline()
	{
		mVertexBuffer.Create(mDevice->GetPhysicalDevice().device, mDevice->GetLogicalDevice().device, mDevice->GetLogicalDevice().transferQueue, mTransferCommandPool.GetVulkanCommandPool(), nullptr, VERTICES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		mIndexBuffer.Create(mDevice->GetPhysicalDevice().device, mDevice->GetLogicalDevice().device, mDevice->GetLogicalDevice().transferQueue, mTransferCommandPool.GetVulkanCommandPool(), nullptr, INDICES, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		mPipelineLayout.Create(mDevice->GetLogicalDevice().device, nullptr)
			.Configure();

		mPipeline.Create(mDevice->GetLogicalDevice().device, nullptr)
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "../../assets/shaders/vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "../../assets/shaders/frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(Vertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(Vertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->GetSwapchainExtent())
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
			.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
			.AddColorBlendAttachmentState(VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
			.ConfigureColorBlend(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
			.Configure(mPipelineLayout.GetVulkanPipelineLayout(), mRenderpass.GetVulkanRenderPass(), 0, 0);

		RecordCommands(0);
	}

	void AnimationKeyframes::CleanupScene()
	{
		mPipelineLayout.Cleanup();
	}

	void AnimationKeyframes::Draw()
	{
		// 1# Get the next available image to draw to and set something to signal when the image is ready to draw to (a semaphore)
		// 2# Submit command buffer to queue for execution, making sure it waits for the image to be available before drawing and signal when it has finished rendering to the image
		// 3# present image to screen when it has signalled finished rendering.

		// wait for given fence to signal (open) from last draw before continuing
		vkWaitForFences(mDevice->GetLogicalDevice().device, 1, &mSynchronizationPrimitives.GetDrawFence(mCurrentFrame), VK_TRUE, std::numeric_limits<uint64_t>::max());
		// close the fense for current frame
		vkResetFences(mDevice->GetLogicalDevice().device, 1, &mSynchronizationPrimitives.GetDrawFence(mCurrentFrame));

		// 1# Get next image and signal when the image is available for drawing
		unsigned int imageIndex;
		vkAcquireNextImageKHR(mDevice->GetLogicalDevice().device, mSwapchain->GetVulkanSwapChain(), std::numeric_limits<uint64_t>::max(), mSynchronizationPrimitives.GetImageAvailableSemaphore(mCurrentFrame), VK_NULL_HANDLE, &imageIndex);

		//RecordCommands(imageIndex);
		//UpdateUniformBuffers(imageIndex);

		// 2# submit command buffer to render
		// Queue submission information
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &mSynchronizationPrimitives.GetImageAvailableSemaphore(mCurrentFrame);
		VkPipelineStageFlags waitFlags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		submitInfo.pWaitDstStageMask = waitFlags;							// stages to check semaphore at
		submitInfo.commandBufferCount = 1;									// number of command buffer to submit
		submitInfo.pCommandBuffers = &mGraphicsCommandBuffers[imageIndex];			// command buffer to submit
		submitInfo.signalSemaphoreCount = 1;								// number of semaphores to signal
		submitInfo.pSignalSemaphores = &mSynchronizationPrimitives.GetRenderFinishedSemaphore(mCurrentFrame);	// semaphore to signal when the command buffer finishes

		// submit command buffer to queue
		VkResult result = vkQueueSubmit(mDevice->GetLogicalDevice().graphicsQueue, 1, &submitInfo, mSynchronizationPrimitives.GetDrawFence(mCurrentFrame));
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit command buffer to queue!!");
		}

		// 3# Present rendered image to screen
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;											// number of semaphore to wait on
		presentInfo.pWaitSemaphores = &mSynchronizationPrimitives.GetRenderFinishedSemaphore(mCurrentFrame);			// semaphore to wait on
		presentInfo.swapchainCount = 1;												// number of swapchain to present to
		presentInfo.pSwapchains = &mSwapchain->GetVulkanSwapChain();										// swapchain to present to
		presentInfo.pImageIndices = &imageIndex;									// index of image in swapchain to present

		result = vkQueuePresentKHR(mDevice->GetLogicalDevice().presentationQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present image!!");
		}

		// Get next frame (% keeps value below MAX_FRAME_DRAWS)
		mCurrentFrame = (mCurrentFrame + 1) % SynchronizationPrimitives::MAX_FRAME_DRAWS;
	}

	void AnimationKeyframes::RecordCommands(const unsigned int& currentImage)
	{
		// Information about how to begin each command buffer
		VkCommandBufferBeginInfo bufferBeginInfo = {};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// not needed any more as we are using fences to synchronise
		//bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	// buffer can be resubmitted when it has already been submitted and waiting for its execution

		// Information about how to begin a render pass (only needed if we are doing a graphical application)
		VkRenderPassBeginInfo renderpassBeginInfo = {};
		renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassBeginInfo.renderPass = mRenderpass.GetVulkanRenderPass();			// render pass to begin
		renderpassBeginInfo.renderArea.offset = { 0, 0 };		// start point of render pass in pixels
		renderpassBeginInfo.renderArea.extent = mSwapchain->GetSwapchainExtent();	// size of region to run render pass on (starting at offset)

		std::array<VkClearValue, 2> clearValues = {};
		//clearValues[0].color = { 0.6f, 0.65f, 0.4f, 1.0f };
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil.depth = 1.0f;

		renderpassBeginInfo.pClearValues = clearValues.data();			// list of clear values (TODO: depth attachment clear value)
		renderpassBeginInfo.clearValueCount = static_cast<unsigned int> (clearValues.size());

		size_t size = mGraphicsCommandBuffers.Size();
		for (size_t i = 0; i < size; ++i)
		{
			renderpassBeginInfo.framebuffer = mFramebuffers[i];

			// 1# Starts recording commands to command buffer
			// 2# Being render pass
			// 3# Bind pipeline
			// 4# draw
			// 5# end render pass
			// 6# end recording commands to command buffer

			// start recording commands to command buffer
			if (vkBeginCommandBuffer(mGraphicsCommandBuffers[i], &bufferBeginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to start recording a command buffer!!");
			}

			// being render pass
			vkCmdBeginRenderPass(mGraphicsCommandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// bind pipeline to be used in render pass
			vkCmdBindPipeline(mGraphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.GetVulkanPipeline());

			VkBuffer vertexBuffers[] = { mVertexBuffer.GetVulkanBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(mGraphicsCommandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(mGraphicsCommandBuffers[i], mIndexBuffer.GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);

			//vkCmdDraw(mGraphicsCommandBuffers[i], 3, 1, 0, 0);
			vkCmdDrawIndexed(mGraphicsCommandBuffers[i], static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);

			// end render pass
			vkCmdEndRenderPass(mGraphicsCommandBuffers[i]);

			if (vkEndCommandBuffer(mGraphicsCommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to stop recording a command buffer!!");
			}
		}
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation (Key Frames)");
	}
}