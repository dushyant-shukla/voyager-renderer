#include "VulkanBasics.h"
#include "graphics/vulkan/ShaderModule.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"
#include "graphics/vulkan/SynchronizationPrimitives.h"
#include "graphics/vulkan/utility/MemoryUtility.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace vr
{
	VulkanBasics::VulkanBasics(std::string name) : Application(name),
		mPipeline(),
		mPipelineLayout()
	{}

	VulkanBasics::~VulkanBasics()
	{
		if (isReady)
		{
			for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); i++) {
				vkDestroyBuffer(mDevice->GetLogicalDevice().device, mMvpBuffers[i], nullptr);
				vkFreeMemory(mDevice->GetLogicalDevice().device, mMvpBuffersMemory[i], nullptr);
			}
			RENDERER_DEBUG("RESOURCE DESTROYED: UNIFORM BUFFER");
			RENDERER_DEBUG("RESOURCE FREED: UNIFORM BUFFER MEMORY");
		}
	}

	void VulkanBasics::InitializeScene()
	{
		mVertexBuffer.Create(VERTICES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		mIndexBuffer.Create(INDICES, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		mMvpBuffers.resize(mSwapchain->GetSwapchainImages().size());
		mMvpBuffersMemory.resize(mSwapchain->GetSwapchainImages().size());
		for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				nullptr, &mMvpBuffers[i], &mMvpBuffersMemory[i]);
		}

		mCross.LoadFromFile("statue.jpg");
		mTextureSampler.CreateDefault();

		SetupDescriptors();

		SetupPipeline();

		//RecordCommands(0);

		isReady = true;
	}

	void VulkanBasics::SetupPipeline()
	{
		mPipelineLayout
			.AddDescriptorSetLayout(mDescriptorSetLayout.GetVkDescriptorSetLayout())
			.Configure();

		mPipeline
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "../../assets/shaders/vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "../../assets/shaders/frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(Vertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(Vertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->GetSwapchainExtent())
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
			.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
			.AddColorBlendAttachmentState(VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
			.ConfigureColorBlendState(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
			.ConfigureDefaultDepthTesting()
			.Configure(mPipelineLayout.GetVulkanPipelineLayout(), mRenderpass.GetVulkanRenderPass(), 0, 0);
	}

	void VulkanBasics::CleanupScene()
	{
	}

	void VulkanBasics::Draw(const double& frametime)
	{
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

		// Get next frame (% keeps value below MAX_FRAME_DRAWS)
		mCurrentFrame = (mCurrentFrame + 1) % SynchronizationPrimitives::MAX_FRAME_DRAWS;
	}

	VkPhysicalDeviceFeatures VulkanBasics::CheckRequiredFeatures()
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

	void VulkanBasics::RecordCommands(const unsigned int& currentImage)
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

			vkCmdBindDescriptorSets(mGraphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout.GetVulkanPipelineLayout(), 0, 1, &mDescriptorSets[i], 0, nullptr);

			//vkCmdDraw(mGraphicsCommandBuffers[i], 3, 1, 0, 0);
			vkCmdDrawIndexed(mGraphicsCommandBuffers[i], static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);

			DrawUI(mGraphicsCommandBuffers[i]);

			// end render pass
			vkCmdEndRenderPass(mGraphicsCommandBuffers[i]);

			if (vkEndCommandBuffer(mGraphicsCommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to stop recording a command buffer!!");
			}
		}
	}

	void VulkanBasics::UpdateUniformBuffer(const unsigned int& index)
	{
		float timeElapsed = mClock.Mark();

		//RENDERER_TRACE("CLOCK DT: {0}", timeElapsed);

		//mvpBuffer.model = glm::mat4(1.0f);
		mvpBuffer.model = glm::rotate(glm::mat4(1.0f), timeElapsed * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		mvpBuffer.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//mvpBuffer.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		mvpBuffer.projection = glm::perspective(glm::radians(45.0f), (float)mSwapchain->GetSwapchainExtent().width / (float)mSwapchain->GetSwapchainExtent().height, 0.1f, 10.0f);
		mvpBuffer.projection[1][1] *= -1;

		void* data;
		vkMapMemory(mDevice->GetLogicalDevice().device, mMvpBuffersMemory[index], 0, sizeof(mvpBuffer), 0, &data);
		memcpy(data, &mvpBuffer, sizeof(mvpBuffer));
		vkUnmapMemory(mDevice->GetLogicalDevice().device, mMvpBuffersMemory[index]);
		//mClock.Reset();
	}

	void VulkanBasics::SetupDescriptors()
	{
		mDescriptorSetLayout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
			.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)
			.Create(0, nullptr);

		mDescriptorPool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<unsigned int>(mSwapchain->GetSwapchainImages().size()))
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, /*static_cast<unsigned int>(mSwapchain->GetSwapchainImages().size())*/1)
			.Create(0, static_cast<unsigned int>(mSwapchain->GetSwapchainImages().size()), nullptr);

		std::vector<VkDescriptorSetLayout> layouts(mSwapchain->GetSwapchainImages().size(), mDescriptorSetLayout.GetVkDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool.GetVulkanDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->GetSwapchainImages().size());
		allocInfo.pSetLayouts = layouts.data();

		mDescriptorSets.resize(mSwapchain->GetSwapchainImages().size());
		CHECK_RESULT(vkAllocateDescriptorSets(mDevice->GetLogicalDevice().device, &allocInfo, mDescriptorSets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SETS");

		for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mMvpBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = mCross.GetVulkanImageView();
			imageInfo.sampler = mTextureSampler.GetVulkanSampler();

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = mDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pImageInfo = &imageInfo; // Optional
			descriptorWrites[1].pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(mDevice->GetLogicalDevice().device, static_cast<unsigned int>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanBasics::SetupTextureSampler()
	{
	}

	Application* CreateApplication()
	{
		return new VulkanBasics("Vulkan Basics");
	}
}