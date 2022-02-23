#include "GpuComputing.h"
#include "graphics/vulkan/ShaderModule.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"
#include "graphics/vulkan/SynchronizationPrimitives.h"
#include "graphics/vulkan/utility/MemoryUtility.h"
#include "RendererState.h"
#include "assertions.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <imgui.h>

namespace vr
{
	GpuComputing::GpuComputing(std::string name) : Application(name),
		mPipeline(),
		mPipelineLayout()
	{}

	GpuComputing::~GpuComputing()
	{
	}

	void GpuComputing::InitializeScene()
	{
		SetupDescriptors();
		SetupPipeline();

		// compute related stuff

		//Compute();

		isReady = true;
	}

	void GpuComputing::SetupPipeline()
	{
		//VkShaderModule =

		//VkPipelineShaderStageCreateInfo computeStage = {};
		//computeStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		//computeStage.module = mShaderModules.back().GetShaderModule();
		//computeStage.pName = "main";

		//VkComputePipelineCreateInfo createPipelineInfo = {};
		//createPipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		//createPipelineInfo.stage = computeStage;
		//createPipelineInfo.layout = layout;
		//createPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		//createPipelineInfo.basePipelineIndex = -1;
		//VkResult result = vkCreateComputePipelines(LOGICAL_DEVICE, VK_NULL_HANDLE, 1, &createPipelineInfo, NULL, &mComputePipeline);
		//CHECK_RESULT(result, "Unable to create a compute pipeline!");
	}

	void GpuComputing::CleanupScene()
	{
	}

	void GpuComputing::Draw(const double& frametime)
	{
		#if 1
		// 1# Get the next available image to draw to and set something to signal when the image is ready to draw to (a semaphore)
		// 2# Submit command buffer to queue for execution, making sure it waits for the image to be available before drawing and signal when it has finished rendering to the image
		// 3# present image to screen when it has signalled finished rendering.

		// wait for given fence to signal (open) from last draw before continuing
		vkWaitForFences(LOGICAL_DEVICE, 1, &mSyncPrimitives.mDrawFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		// close the fense for current frame
		vkResetFences(LOGICAL_DEVICE, 1, &mSyncPrimitives.mDrawFences[mCurrentFrame]);

		// 1# Get next image and signal when the image is available for drawing
		unsigned int imageIndex;
		vkAcquireNextImageKHR(mDevice->GetLogicalDevice().device, mSwapchain->GetVulkanSwapChain(), std::numeric_limits<uint64_t>::max(), mSyncPrimitives.GetImageAvailableSemaphore(mCurrentFrame), VK_NULL_HANDLE, &imageIndex);

		//RecordCommands(imageIndex);
		RecordCommands(0);
		UpdateUniformBuffer(imageIndex);

		// 2# submit command buffer to render
		// Queue submission information
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &mSyncPrimitives.mImageAvailable[mCurrentFrame];
		VkPipelineStageFlags waitFlags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		submitInfo.pWaitDstStageMask = waitFlags;							// stages to check semaphore at
		submitInfo.commandBufferCount = 1;									// number of command buffer to submit
		submitInfo.pCommandBuffers = &mGraphicsCommandBuffers[imageIndex];			// command buffer to submit
		submitInfo.signalSemaphoreCount = 1;								// number of semaphores to signal
		submitInfo.pSignalSemaphores = &mSyncPrimitives.mRenderFinished[mCurrentFrame];	// semaphore to signal when the command buffer finishes

		// submit command buffer to queue
		VkResult result = vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, mSyncPrimitives.mDrawFences[mCurrentFrame]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit command buffer to queue!!");
		}

		// 3# Present rendered image to screen
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;											// number of semaphore to wait on
		presentInfo.pWaitSemaphores = &mSyncPrimitives.mRenderFinished[mCurrentFrame];			// semaphore to wait on
		presentInfo.swapchainCount = 1;												// number of swapchain to present to
		presentInfo.pSwapchains = &mSwapchain->GetVulkanSwapChain();										// swapchain to present to
		presentInfo.pImageIndices = &imageIndex;									// index of image in swapchain to present

		result = vkQueuePresentKHR(mDevice->GetLogicalDevice().presentationQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present image!!");
		}
		#endif

		// Get next frame (% keeps value below MAX_FRAME_DRAWS)
		mCurrentFrame = (mCurrentFrame + 1) % SynchronizationPrimitives::MAX_FRAME_DRAWS;
	}

	VkPhysicalDeviceFeatures GpuComputing::CheckRequiredFeatures()
	{
		bool requiredFeaturesAvailable = true;
		VkPhysicalDeviceFeatures requiredDeviceFeatures = {};
		if (!mDevice->GetPhysicalDevice().features.samplerAnisotropy)
		{
			requiredFeaturesAvailable = false;
		}

		if (requiredFeaturesAvailable)
		{
			requiredDeviceFeatures.samplerAnisotropy = VK_TRUE;
			return requiredDeviceFeatures;
		}

		throw std::runtime_error("DEVICE NOT SUITABLE: REQUIRED FEATURES NOT AVAILABLE");
	}

	void GpuComputing::RecordCommands(const unsigned int& currentImage)
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
			//vkCmdBindPipeline(mGraphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.GetVulkanPipeline());

			DrawUI(mGraphicsCommandBuffers[i]);

			// end render pass
			vkCmdEndRenderPass(mGraphicsCommandBuffers[i]);

			if (vkEndCommandBuffer(mGraphicsCommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to stop recording a command buffer!!");
			}
		}
	}

	void GpuComputing::UpdateUniformBuffer(const unsigned int& index)
	{
	}

	void GpuComputing::SetupDescriptors()
	{
	}

	void GpuComputing::SetupTextureSampler()
	{
	}

	void GpuComputing::Compute()
	{
		PrepareComputeCommandBuffer();
		BeginComputeCommandBuffer();
		RecordComputeCommands();
		EndComputerCommandBuffer();
	}

	void GpuComputing::PrepareComputeCommandBuffer()
	{
		mComputeCommandBuffer.Create(mDevice->GetPhysicalDevice().queueFamilies.compute.value(), 1);
	}

	void GpuComputing::BeginComputeCommandBuffer()
	{
		VkCommandBufferBeginInfo bufferBeginInfo;
		memset(&bufferBeginInfo, 0, sizeof(bufferBeginInfo));
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		ASSERT_SUCCESS_AND_THROW(vkBeginCommandBuffer(mComputeCommandBuffer[0], &bufferBeginInfo), "Failed to begin compute command buffer!");
	}

	void GpuComputing::RecordComputeCommands()
	{
		vkCmdBindPipeline(mComputeCommandBuffer[0], VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);
		vkCmdDispatch(mComputeCommandBuffer[0], 1, 1, 1);
	}

	void GpuComputing::EndComputerCommandBuffer()
	{
		ASSERT_SUCCESS_AND_THROW(vkEndCommandBuffer(mComputeCommandBuffer[0]), "Failed to end recoding compute command buffer!");
	}

	void GpuComputing::OnUpdateUIOverlay(UiOverlay* overlay)
	{
		ImGui::SetNextWindowPos(ImVec2(10, 210));
		ImGui::Begin("Gpu Computing");
		ImGui::End();
	}

	Application* CreateApplication()
	{
		return new GpuComputing("Gpu Computing");
	}
}