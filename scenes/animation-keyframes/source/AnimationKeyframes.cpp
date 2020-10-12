#include "AnimationKeyframes.h"
#include "RendererState.h"

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name)
	{
	}

	AnimationKeyframes::~AnimationKeyframes()
	{
		if (isReady)
		{
			for (auto& model : mModels)
			{
				delete model;
			}

			for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); i++) {
				vkDestroyBuffer(LOGICAL_DEVICE, mViewUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, mViewUboBuffersMemory[i], nullptr);
			}
			RENDERER_DEBUG("RESOURCE DESTROYED: UNIFORM BUFFER");
			RENDERER_DEBUG("RESOURCE FREED: UNIFORM BUFFER MEMORY");
		}
	}

	void AnimationKeyframes::InitializeScene()
	{
		SetupTextureSampler();
		LoadAssets();
		SetupUniformBufferObjects();

		mViewUBO.projectionViewMatrix = glm::perspective(glm::radians(45.0f),
			(float)mSwapchain->mExtent.width / (float)mSwapchain->mExtent.height,
			0.1f, 100.0f);
		mViewUBO.projectionViewMatrix[1][1] *= -1;
		mViewUBO.projectionViewMatrix *= glm::lookAt(glm::vec3(20.0f, 20.0f, 60.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		mPerModelData.model = model;

		SetupDescriptorSets();
		SetupPipeline();
		isReady = true;
	}

	void AnimationKeyframes::CleanupScene()
	{
	}

	void AnimationKeyframes::Draw()
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
		vkAcquireNextImageKHR(LOGICAL_DEVICE, mSwapchain->mSwapchain, std::numeric_limits<uint64_t>::max(), mSyncPrimitives.mImageAvailable[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

		RecordCommands(imageIndex);
		UpdateUniformBuffers(imageIndex);

		// 2# submit command buffer to render
		// Queue submission information
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
		presentInfo.pWaitSemaphores = &mSyncPrimitives.mRenderFinished[mCurrentFrame];			// semaphore to wait on
		presentInfo.swapchainCount = 1;																		// number of swapchain to present to
		presentInfo.pSwapchains = &mSwapchain->mSwapchain;										// swapchain to present to
		presentInfo.pImageIndices = &imageIndex;															// index of image in swapchain to present

		result = vkQueuePresentKHR(mDevice->GetLogicalDevice().presentationQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present image!!");
		}

		//vkQueueWaitIdle(mDevice->GetLogicalDevice().presentationQueue);
		// Get next frame (% keeps value below MAX_FRAME_DRAWS)
		mCurrentFrame = (mCurrentFrame + 1) % SynchronizationPrimitives::MAX_FRAME_DRAWS;
	}

	VkPhysicalDeviceFeatures AnimationKeyframes::CheckRequiredFeatures()
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

	void AnimationKeyframes::SetupPipeline()
	{
		// pipeline layout
		//								1. descriptor layout
		//								2. push constant for model matrix
		mPipelineLayouts.pipelineLayout
			.AddDescriptorSetLayout(mDescriptors.layout.GetVkDescriptorSetLayout())
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPerModelData))
			.Configure();

		// setup pipeline
		mPipelines.pipeline // TODO setup shaders
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "../../assets/shaders/vertex-skinning/vertex-skinning.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "../../assets/shaders/vertex-skinning/vertex-skinning.frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(vrassimp::Vertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(vrassimp::Vertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->GetSwapchainExtent())
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
			.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
			.AddColorBlendAttachmentState(VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
			.ConfigureColorBlend(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
			.Configure(mPipelineLayouts.pipelineLayout.GetVulkanPipelineLayout(), mRenderpass.GetVulkanRenderPass(), 0, 0);
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
		renderpassBeginInfo.renderArea.extent = mSwapchain->mExtent;	// size of region to run render pass on (starting at offset)

		std::array<VkClearValue, 2> clearValues = {};
		//clearValues[0].color = { 0.6f, 0.65f, 0.4f, 1.0f };
		clearValues[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		clearValues[1].depthStencil.depth = 1.0f;

		renderpassBeginInfo.pClearValues = clearValues.data();			// list of clear values (TODO: depth attachment clear value)
		renderpassBeginInfo.clearValueCount = static_cast<unsigned int> (clearValues.size());

		renderpassBeginInfo.framebuffer = mFramebuffers[currentImage];

		// 1# Starts recording commands to command buffer
		// 2# Being render pass
		// 3# Bind pipeline
		// 4# draw
		// 5# end render pass
		// 6# end recording commands to command buffer

		// start recording commands to command buffer
		if (vkBeginCommandBuffer(mGraphicsCommandBuffers[currentImage], &bufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to start recording a command buffer!!");
		}

		// being render pass
		vkCmdBeginRenderPass(mGraphicsCommandBuffers[currentImage], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// bind pipeline to be used in render pass
		vkCmdBindPipeline(mGraphicsCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.pipeline.GetVulkanPipeline());

		for (size_t i = 0; i < mModels.size(); ++i)
		{
			vrassimp::Model* currentModel = mModels[i];

			// TODO: Push Constants
			// all meshes within a model will have the same model matrix
			vkCmdPushConstants(mGraphicsCommandBuffers[currentImage],
				mPipelineLayouts.pipelineLayout.GetVulkanPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(mPerModelData), &mPerModelData);	// we can also pass an array of data

			for (size_t j = 0; j < currentModel->meshes.size(); ++j)
			{
				vkCmdBindDescriptorSets(mGraphicsCommandBuffers[currentImage],
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					mPipelineLayouts.pipelineLayout.GetVulkanPipelineLayout(),
					0,
					1, &mDescriptors.sets[currentImage], 0, nullptr);

				vrassimp::Mesh* currentMesh = currentModel->meshes[j];
				currentMesh->Draw(mGraphicsCommandBuffers[currentImage]); // bind vertex and index buffer, cmdIndexedDraw()
			}
		}

		// end render pass
		vkCmdEndRenderPass(mGraphicsCommandBuffers[currentImage]);

		if (vkEndCommandBuffer(mGraphicsCommandBuffers[currentImage]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop recording a command buffer!!");
		}
	}

	void AnimationKeyframes::SetupDescriptorSets()
	{
		mDescriptors.pool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.Create(0, static_cast<unsigned> (mSwapchain->GetSwapchainImages().size()), nullptr);

		mDescriptors.layout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)				// view projection ubo
			.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler diffuse
			.AddLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler specular
			.AddLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler emission
			.Create(0, nullptr);

		// Allocating descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(mSwapchain->GetSwapchainImages().size(), mDescriptors.layout.GetVkDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptors.pool.GetVulkanDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->GetSwapchainImages().size());
		allocInfo.pSetLayouts = layouts.data();
		mDescriptors.sets.resize(mSwapchain->GetSwapchainImages().size());
		CHECK_RESULT(vkAllocateDescriptorSets(mDevice->GetLogicalDevice().device, &allocInfo, mDescriptors.sets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SETS");

		// configuring allocated descriptor sets with buffer and image information
		for (size_t i = 0; i < mDescriptors.sets.size(); ++i)
		{
			std::vector<VkWriteDescriptorSet> descriptorWrites = {};

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = mViewUboBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(mViewUBO);

			VkWriteDescriptorSet writeBufferInfo = {};
			writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeBufferInfo.dstSet = mDescriptors.sets[i];
			writeBufferInfo.dstBinding = 0;
			writeBufferInfo.dstArrayElement = 0;
			writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeBufferInfo.descriptorCount = 1;
			writeBufferInfo.pBufferInfo = &bufferInfo;
			writeBufferInfo.pImageInfo = nullptr; // Optional
			writeBufferInfo.pTexelBufferView = nullptr; // Optional

			descriptorWrites.push_back(writeBufferInfo);

			for (size_t j = 0; j < imageInfos.size(); ++j)
			{
				VkWriteDescriptorSet writeImageInfo = {};
				writeImageInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeImageInfo.dstSet = mDescriptors.sets[i];
				writeImageInfo.dstBinding = imageInfos[j].binding;
				writeImageInfo.dstArrayElement = 0;
				writeImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeImageInfo.descriptorCount = 1;
				writeImageInfo.pBufferInfo = nullptr;
				writeImageInfo.pImageInfo = &imageInfos[j].info; // Optional
				writeImageInfo.pTexelBufferView = nullptr; // Optional

				descriptorWrites.push_back(writeImageInfo);
			}

			vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void AnimationKeyframes::SetupUniformBufferObjects()
	{
		VkDeviceSize bufferSize = sizeof(mViewUBO);
		mViewUboBuffers.resize(mSwapchain->GetSwapchainImages().size());
		mViewUboBuffersMemory.resize(mSwapchain->GetSwapchainImages().size());
		for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				nullptr, &mViewUboBuffers[i], &mViewUboBuffersMemory[i]);
		}
	}

	void AnimationKeyframes::SetupTextureSampler()
	{
		mSamplers.diffuse.CreateDefault();
		mSamplers.specular.CreateDefault();
		mSamplers.emission.CreateDefault();
	}

	void AnimationKeyframes::LoadAssets()
	{
		vrassimp::Model::ModelCreateInfo modelCreateInfo(1.0, 1.0f, 0.0f);

		vrassimp::Model* model = new vrassimp::Model();
		model->LoadFromFile("wolf\\scene.gltf", &modelCreateInfo);
		mModels.push_back(model);

		/*
			Setup meshes' vertex buffers, index buffers and textures
		*/
		for (size_t modelIndex = 0; modelIndex < mModels.size(); ++modelIndex)
		{
			for (size_t meshIndex = 0; meshIndex < mModels[modelIndex]->meshes.size(); ++meshIndex)
			{
				mModels[modelIndex]->meshes[meshIndex]->buffers.vertex.Create(mModels[modelIndex]->meshes[meshIndex]->vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				mModels[modelIndex]->meshes[meshIndex]->buffers.index.Create(mModels[modelIndex]->meshes[meshIndex]->indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

				for (size_t textureIndex = 0; textureIndex < mModels[modelIndex]->meshes[meshIndex]->textures.size(); ++textureIndex)
				{
					vrassimp::Texture* texture = mModels[modelIndex]->meshes[meshIndex]->textures[textureIndex];
					Texture* t;
					ImageInfo info;
					switch (texture->type)
					{
					case vrassimp::Texture::Type::DIFFUSE:
						t = new Texture();
						t->LoadFromFile(texture->path.c_str(), mSamplers.diffuse.GetVulkanSampler());
						texture->texture = t;
						info = { 1, t->mImageInfo };
						imageInfos.push_back(info);
						break;
					case vrassimp::Texture::Type::SPECULAR:
						t = new Texture();
						t->LoadFromFile(texture->path.c_str(), mSamplers.specular.GetVulkanSampler());
						texture->texture = t;
						info = { 2, t->mImageInfo };
						imageInfos.push_back(info);
						break;
					case vrassimp::Texture::Type::EMISSIVE:
						t = new Texture();
						t->LoadFromFile(texture->path.c_str(), mSamplers.emission.GetVulkanSampler());
						texture->texture = t;
						info = { 3, t->mImageInfo };
						imageInfos.push_back(info);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	void AnimationKeyframes::UpdateUniformBuffers(const unsigned int& imageIndex)
	{
		void* data;
		vkMapMemory(LOGICAL_DEVICE, mViewUboBuffersMemory[imageIndex], 0, sizeof(mViewUBO), 0, &data);
		memcpy(data, &mViewUBO, sizeof(mViewUBO));
		vkUnmapMemory(LOGICAL_DEVICE, mViewUboBuffersMemory[imageIndex]);
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation Key-frames");
	}
}