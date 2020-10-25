#include "MotionAlongPath.h"
#include "CurveVertex.h"
#include "graphics/vulkan/utility/RendererCoreUtility.h"

namespace vr
{
	MotionAlongPath::MotionAlongPath(std::string name) : Application(name)
	{
		eCamera.type = EditingModeCamera::Type::look_at;
		eCamera.matrices.flipY = true;
		eCamera.SetPosition(glm::vec3(0.0f, 0.75f, -2.0f));
		//eCamera.SetPosition(glm::vec3(-40.0f, 9.0f, 18.0f));
		eCamera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		eCamera.SetPerspective(glm::radians(60.0f), (float)Window::WIDTH / Window::HEIGHT, 0.1f, 1024.0f);
	}

	MotionAlongPath::~MotionAlongPath()
	{
		if (isReady)
		{
			for (size_t i = 0; i < mSwapchain->mImages.size(); i++)
			{
				vkDestroyBuffer(LOGICAL_DEVICE, viewUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, viewUboMemory[i], nullptr);
			}
			RENDERER_DEBUG("RESOURCE DESTROYED: UNIFORM BUFFER");
			RENDERER_DEBUG("RESOURCE FREED: UNIFORM BUFFER MEMORY");

			delete curve;
		}
	}

	void MotionAlongPath::InitializeScene()
	{
		// curve
		curve = new Splines();
		SetupDescriptorSetForCurve();
		SetupPipelineForCurve();

		isReady = true;
	}

	void MotionAlongPath::CleanupScene()
	{
	}

	void MotionAlongPath::Draw(const double& frametime)
	{
		vkWaitForFences(LOGICAL_DEVICE, 1, &mSyncPrimitives.mDrawFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(LOGICAL_DEVICE, 1, &mSyncPrimitives.mDrawFences[mCurrentFrame]);

		unsigned int imageIndex;
		vkAcquireNextImageKHR(LOGICAL_DEVICE, mSwapchain->mSwapchain, std::numeric_limits<uint64_t>::max(), mSyncPrimitives.mImageAvailable[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

		RecordCommands(imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &mSyncPrimitives.mImageAvailable[mCurrentFrame];
		VkPipelineStageFlags waitFlags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		submitInfo.pWaitDstStageMask = waitFlags;													// stages to check semaphore at
		submitInfo.commandBufferCount = 1;															// number of command buffer to submit
		submitInfo.pCommandBuffers = &mGraphicsCommandBuffers[imageIndex];							// command buffer to submit
		submitInfo.signalSemaphoreCount = 1;														// number of semaphores to signal
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
		presentInfo.waitSemaphoreCount = 1;																	// number of semaphore to wait on
		presentInfo.pWaitSemaphores = &mSyncPrimitives.mRenderFinished[mCurrentFrame];						// semaphore to wait on
		presentInfo.swapchainCount = 1;																		// number of swapchain to present to
		presentInfo.pSwapchains = &mSwapchain->mSwapchain;													// swapchain to present to
		presentInfo.pImageIndices = &imageIndex;															// index of image in swapchain to present

		result = vkQueuePresentKHR(mDevice->GetLogicalDevice().presentationQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present image!!");
		}

		//vkQueueWaitIdle(mDevice->GetLogicalDevice().presentationQueue);
		// Get next frame (% keeps value below MAX_FRAME_DRAWS)
		mCurrentFrame = (mCurrentFrame + 1) % SynchronizationPrimitives::MAX_FRAME_DRAWS;

		//timer += frametime * animationSettings.speed;
	}

	VkPhysicalDeviceFeatures MotionAlongPath::CheckRequiredFeatures()
	{
		bool requiredFeaturesAvailable = true;
		VkPhysicalDeviceFeatures requiredDeviceFeatures = {};
		if (!(mDevice->GetPhysicalDevice().features.samplerAnisotropy && mDevice->GetPhysicalDevice().features.largePoints && mDevice->GetPhysicalDevice().features.wideLines))
		{
			requiredFeaturesAvailable = false;
		}

		if (requiredFeaturesAvailable)
		{
			requiredDeviceFeatures.samplerAnisotropy = VK_TRUE;
			requiredDeviceFeatures.largePoints = VK_TRUE;
			requiredDeviceFeatures.wideLines = VK_TRUE;
			return requiredDeviceFeatures;
		}

		throw std::runtime_error("DEVICE NOT SUITABLE: REQUIRED FEATURES NOT AVAILABLE");
	}

	void MotionAlongPath::SetupDescriptorSetForCurve()
	{
		SetupUboForCurve();

		mDescriptorPools.viewUBO
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.Create(0, static_cast<unsigned int>(mSwapchain->mImages.size()), nullptr);

		mDescriptorSetLayouts.viewUBO
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
			.Create(0, nullptr);

		mDescriptorSets.curve.Setup(mDescriptorSetLayouts.viewUBO.mLayout, mDescriptorPools.viewUBO.mPool, static_cast<unsigned int>(mSwapchain->mImages.size()));

		std::vector<VkWriteDescriptorSet> descriptorWrites = {};
		for (size_t i = 0; i < mDescriptorSets.curve.mSets.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = viewUboBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(viewUBO);

			VkWriteDescriptorSet writeBufferInfo = {};
			writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeBufferInfo.dstSet = mDescriptorSets.curve.mSets[i];
			writeBufferInfo.dstBinding = 0;
			writeBufferInfo.dstArrayElement = 0;
			writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeBufferInfo.descriptorCount = 1;
			writeBufferInfo.pBufferInfo = &bufferInfo;
			writeBufferInfo.pImageInfo = nullptr; // Optional
			writeBufferInfo.pTexelBufferView = nullptr; // Optional

			descriptorWrites.push_back(writeBufferInfo);
		}
		vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void MotionAlongPath::SetupPipelineForCurve()
	{
		mPipelineLayouts.curve
			.AddDescriptorSetLayout(mDescriptorSetLayouts.viewUBO.mLayout)
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(curvePushConstant))
			.Configure();

		mPipelines.curve
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "motion-along-path/primitive-drawing.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "motion-along-path/primitive-drawing.frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(CurveVertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(CurveVertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->mExtent)
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
			.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
			.ConfigureDefaultDepthTesting()
			.AddColorBlendAttachmentState(VK_FALSE,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_SUBTRACT,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
			.ConfigureColorBlendState(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
			.Configure(mPipelineLayouts.curve.mLayout, mRenderpass.mRenderPass, 0, 0);
	}

	void MotionAlongPath::SetupUboForCurve()
	{
		VkDeviceSize bufferSize = sizeof(viewUBO);
		viewUboBuffers.resize(mSwapchain->mImages.size());
		viewUboMemory.resize(mSwapchain->mImages.size());
		for (unsigned int i = 0; i < mSwapchain->mImages.size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ALLOCATION_CALLBACK, &viewUboBuffers[i], &viewUboMemory[i]);
		}
	}

	void MotionAlongPath::UpdateUniformBufferForCurve(unsigned int imageIndex)
	{
		viewUBO.projection = eCamera.matrices.projection;
		viewUBO.view = eCamera.matrices.view;

		void* data;
		vkMapMemory(LOGICAL_DEVICE, viewUboMemory[imageIndex], 0, sizeof(viewUBO), 0, &data);
		memcpy(data, &viewUBO, sizeof(viewUBO));
		vkUnmapMemory(LOGICAL_DEVICE, viewUboMemory[imageIndex]);
	}

	void MotionAlongPath::RecordCommands(unsigned int imageIndex)
	{
		// Information about how to begin each command buffer
		VkCommandBufferBeginInfo bufferBeginInfo = {};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Information about how to begin a render pass (only needed if we are doing a graphical application)
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		clearValues[1].depthStencil.depth = 1.0f;
		VkRenderPassBeginInfo renderpassBeginInfo = {};
		renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassBeginInfo.renderPass = mRenderpass.GetVulkanRenderPass();			// render pass to begin
		renderpassBeginInfo.renderArea.offset = { 0, 0 };		// start point of render pass in pixels
		renderpassBeginInfo.renderArea.extent = mSwapchain->mExtent;	// size of region to run render pass on (starting at offset)
		renderpassBeginInfo.pClearValues = clearValues.data();			// list of clear values (TODO: depth attachment clear value)
		renderpassBeginInfo.clearValueCount = static_cast<unsigned int> (clearValues.size());
		renderpassBeginInfo.framebuffer = mFramebuffers[imageIndex];

		// begin command buffer
		CHECK_RESULT(vkBeginCommandBuffer(mGraphicsCommandBuffers[imageIndex], &bufferBeginInfo), "FAILED TO START RECORDING A COMMAND BUFFER!");

		// being render pass
		vkCmdBeginRenderPass(mGraphicsCommandBuffers[imageIndex], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// bind curve pipeline
		// update curve ubo
		// update push constant for curve
		// bind descriptor sets
		// draw curve
		vkCmdBindPipeline(mGraphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.curve.GetVulkanPipeline());

		UpdateUniformBufferForCurve(imageIndex);

		curvePushConstant.model = glm::mat4(1.0f);
		//curvePushConstant.model = glm::scale(curvePushConstant.model, glm::vec3(2.0f, 2.0f, 2.0f));
		vkCmdPushConstants(mGraphicsCommandBuffers[imageIndex],
			mPipelineLayouts.curve.mLayout,
			VK_SHADER_STAGE_VERTEX_BIT, 0,
			sizeof(curvePushConstant), &curvePushConstant);

		std::vector<VkDescriptorSet> descriptorSets = { mDescriptorSets.curve.mSets[imageIndex] };
		vkCmdBindDescriptorSets(mGraphicsCommandBuffers[imageIndex],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayouts.curve.mLayout,
			0,
			static_cast<unsigned int>(descriptorSets.size()),
			descriptorSets.data(), 0, nullptr);
		curve->DrawSpline(mGraphicsCommandBuffers[imageIndex]);

		// draw models
		// for loop

		DrawUI(mGraphicsCommandBuffers[imageIndex]);

		// end render pass
		vkCmdEndRenderPass(mGraphicsCommandBuffers[imageIndex]);

		if (vkEndCommandBuffer(mGraphicsCommandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop recording a command buffer!!");
		}
	}

	void MotionAlongPath::OnUpdateUIOverlay(UiOverlay* overlay)
	{
	}

	Application* CreateApplication()
	{
		return new MotionAlongPath("Animation Along A Curve");
	}
}