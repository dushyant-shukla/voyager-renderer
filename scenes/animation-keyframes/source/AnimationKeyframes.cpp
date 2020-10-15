#include "AnimationKeyframes.h"
#include "RendererState.h"
#include "window/Window.h"

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name)
	{
		eCamera.type = EditingModeCamera::Type::look_at;
		eCamera.matrices.flipY = true;
		eCamera.SetPosition(glm::vec3(0.0f, 0.75f, -2.0f));
		eCamera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		eCamera.SetPerspective(glm::radians(60.0f), (float)Window::WIDTH / Window::HEIGHT, 0.1f, 1024.0f);

		//animationTimer.emplace_back(0.0f, 2233.33325f); // nathan

		//animationTimer.emplace_back(0.0f, 99.0f); // wolverine

		//animationTimer.emplace_back(0.0f, 95.99f);
		//animationTimer.emplace_back(96.0f, 191.99f);
		//animationTimer.emplace_back(191.99f, 287.98f);
		//animationTimer.emplace_back(287.98f, 384.0f);
		//animationTimer.emplace_back(384.0f, 480.97f);
		//animationTimer.emplace_back(480.97f, 575.97f);
		//animationTimer.emplace_back(575.97f, 671.97f);
	}

	AnimationKeyframes::~AnimationKeyframes()
	{
		if (isReady)
		{
			for (auto& model : mModels)
			{
				delete model;
			}

			for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); i++)
			{
				vkDestroyBuffer(LOGICAL_DEVICE, uboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, uboBuffersMemory[i], nullptr);
			}
			RENDERER_DEBUG("RESOURCE DESTROYED: UNIFORM BUFFER");
			RENDERER_DEBUG("RESOURCE FREED: UNIFORM BUFFER MEMORY");
		}
	}

	void AnimationKeyframes::InitializeScene()
	{
		// do not change the order of setup calls
		SetupTextureSampler();
		SetupUniformBufferObjects();

		SetupDescriptorSets();
		SetupPipeline();

		LoadAssets();

		ubo.projection = eCamera.matrices.projection;
		ubo.view = eCamera.matrices.view;
		ubo.viewPosition = eCamera.orientation.viewPosition;
		UpdateBoneTransforms();

		// NATHAN
		glm::mat4 model(1.0f);/* = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));*/
		model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
		model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.50f)); // tiger
		//model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); // nathan
		//model = glm::scale(model, glm::vec3(3.00f, 3.0f, 3.0f)); // spidey
		mPerModelData.model = model;

		// BLADE
		//glm::mat4 model(1.0f);/* = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));*/
		//model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
		//model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		//mPerModelData.model = model;

		// wolf
		//glm::mat4 model(1.0f);/* = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));*/
		//model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
		//model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//mPerModelData.model = model;

		isReady = true;
	}

	void AnimationKeyframes::CleanupScene()
	{
	}

	void AnimationKeyframes::Draw(const double& frametime)
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

		timer += frametime * 0.75;
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
			.AddDescriptorSetLayout(mDescriptors.uniformBufferLayout.GetVkDescriptorSetLayout())
			.AddDescriptorSetLayout(mDescriptors.textureLayout.mLayout)
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
			.AddColorBlendAttachmentState(VK_TRUE,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_SUBTRACT,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
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
				vrassimp::Mesh* currentMesh = currentModel->meshes[j];

				std::vector<VkDescriptorSet> descriptorSets = { mDescriptors.uniformBufferSets[currentImage] }; //, currentMesh->mDescriptorSets[0] };

				if (!currentMesh->textures.empty())
				{
					descriptorSets.push_back(currentMesh->mDescriptorSets[0]);
				}

				vkCmdBindDescriptorSets(mGraphicsCommandBuffers[currentImage],
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					mPipelineLayouts.pipelineLayout.GetVulkanPipelineLayout(),
					0,
					static_cast<unsigned int>(descriptorSets.size()),
					descriptorSets.data(), 0, nullptr);

				//vrassimp::Mesh* currentMesh = currentModel->meshes[j];
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
		mDescriptors.uniformBufferPool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.Create(0, static_cast<unsigned> (mSwapchain->GetSwapchainImages().size()), nullptr);

		mDescriptors.uniformBufferLayout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)				// view projection ubo
			.Create(0, nullptr);

		// Allocating descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(mSwapchain->GetSwapchainImages().size(), mDescriptors.uniformBufferLayout.GetVkDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptors.uniformBufferPool.GetVulkanDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->GetSwapchainImages().size());
		allocInfo.pSetLayouts = layouts.data();
		mDescriptors.uniformBufferSets.resize(mSwapchain->GetSwapchainImages().size());
		CHECK_RESULT(vkAllocateDescriptorSets(mDevice->GetLogicalDevice().device, &allocInfo, mDescriptors.uniformBufferSets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SETS");

		// configuring allocated descriptor sets with buffer and image information
		std::vector<VkWriteDescriptorSet> descriptorWrites = {};
		for (size_t i = 0; i < mDescriptors.uniformBufferSets.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uboBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(ubo);

			VkWriteDescriptorSet writeBufferInfo = {};
			writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeBufferInfo.dstSet = mDescriptors.uniformBufferSets[i];
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

		mDescriptors.texturePool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.Create(0, 50, nullptr); // it will be read only resource, a descriptor set does not need to have exclusive write privilege

		mDescriptors.textureLayout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler diffuse
			.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler specular
			.AddLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler emission
			.Create(0, nullptr);
	}

	void AnimationKeyframes::SetupUniformBufferObjects()
	{
		VkDeviceSize bufferSize = sizeof(ubo);
		uboBuffers.resize(mSwapchain->GetSwapchainImages().size());
		uboBuffersMemory.resize(mSwapchain->GetSwapchainImages().size());
		for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				nullptr, &uboBuffers[i], &uboBuffersMemory[i]);
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
		//model->LoadFromFile("blade\\scene.gltf", &modelCreateInfo);
		//model->LoadFromFile("wolf\\scene.gltf", &modelCreateInfo);
		//model->LoadFromFile("alpha-wolf\\alpha-wolf.fbx", &modelCreateInfo);
		//model->LoadFromFile("wolverine\\wolverine.fbx", &modelCreateInfo);
		//model->LoadFromFile("wolverine\\scene.gltf", &modelCreateInfo);
		//model->LoadFromFile("alpha-wolf\\scene.gltf", &modelCreateInfo);
		//model->LoadFromFile("jumping.fbx", &modelCreateInfo);
		//model->LoadFromFile("wolf-ii\\Wolf_dae.dae", &modelCreateInfo);
		//model->LoadFromFile("iron-man-fortnite\\scene.gltf", &modelCreateInfo);
		//model->LoadFromFile("nathan\\scene.gltf", &modelCreateInfo); // works - loads correctly
		model->LoadFromFile("bengal-tiger\\tiger.fbx", &modelCreateInfo); // works - loads correctly
		//model->LoadFromFile("myth-creature\\myth-creature.fbx", &modelCreateInfo); // works - loads correctly
		//model->LoadFromFile("spiderman\\spiderman.fbx", &modelCreateInfo); // works - loads correctly
		mModels.push_back(model);

		/*
			Setup meshes' vertex buffers, index buffers and textures
		*/
		for (size_t modelIndex = 0; modelIndex < mModels.size(); ++modelIndex)
		{
			for (size_t meshIndex = 0; meshIndex < mModels[modelIndex]->meshes.size(); ++meshIndex)
			{
				vrassimp::Mesh* currentMesh = mModels[modelIndex]->meshes[meshIndex];

				currentMesh->buffers.vertex.Create(currentMesh->vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				currentMesh->buffers.index.Create(currentMesh->indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

				if (!currentMesh->textures.empty())
				{
					// allocate a descritor set to hold per mesh data (texture data for now...)
					currentMesh->mDescriptorSets.Setup(mDescriptors.textureLayout.mLayout, mDescriptors.texturePool.mPool, 1);
					std::vector<VkWriteDescriptorSet> descriptorWrites;
					descriptorWrites.reserve(currentMesh->textures.size());

					for (size_t textureIndex = 0; textureIndex < currentMesh->textures.size(); ++textureIndex)
					{
						vrassimp::Texture* texture = mModels[modelIndex]->meshes[meshIndex]->textures[textureIndex];
						Texture* t = nullptr;
						switch (texture->type)
						{
						case vrassimp::Texture::Type::DIFFUSE:
							t = new Texture(0);
							t->LoadFromFile(texture->path.c_str(), mSamplers.diffuse.GetVulkanSampler());
							texture->texture = t;
							break;
						case vrassimp::Texture::Type::SPECULAR:
							t = new Texture(1);
							t->LoadFromFile(texture->path.c_str(), mSamplers.specular.GetVulkanSampler());
							texture->texture = t;
							break;
						case vrassimp::Texture::Type::EMISSIVE:
							t = new Texture(2);
							t->LoadFromFile(texture->path.c_str(), mSamplers.emission.GetVulkanSampler());
							texture->texture = t;
							break;
						default:
							break;
						}

						if (t != nullptr)
						{
							VkWriteDescriptorSet writeImageInfo = {};
							writeImageInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
							writeImageInfo.dstSet = currentMesh->mDescriptorSets[0];
							writeImageInfo.dstBinding = t->mBinding;
							writeImageInfo.dstArrayElement = 0;
							writeImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							writeImageInfo.descriptorCount = 1;
							writeImageInfo.pImageInfo = &t->mImageInfo;

							descriptorWrites.push_back(writeImageInfo);
						}
					}
					vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
				}
			}
		}
	}

	void AnimationKeyframes::UpdateUniformBuffers(const unsigned int& imageIndex)
	{
		ubo.projection = eCamera.matrices.projection;
		ubo.view = eCamera.matrices.view;
		ubo.viewPosition = eCamera.orientation.viewPosition;
		UpdateBoneTransforms();

		void* data;
		vkMapMemory(LOGICAL_DEVICE, uboBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(LOGICAL_DEVICE, uboBuffersMemory[imageIndex]);
	}

	void AnimationKeyframes::UpdateBoneTransforms()
	{
		for (unsigned int i = 0; i < mModels.size(); ++i)
		{
			vrassimp::Animation* animation = mModels[i]->mAnimation;
			if (timer > animation->animationTimes[animation->currentIndex].end)
			{
				animation->currentIndex = (animation->currentIndex + 1) % animation->animationTimes.size();
				RENDERER_INFO("CURRENT ANIMATION: {0}", animation->currentIndex);
				timer = animation->animationTimes[animation->currentIndex].start;
			}
			mModels[i]->mAnimation->BoneTransform(timer, boneTransforms);
		}

		for (unsigned int i = 0; i < boneTransforms.size(); ++i)
		{
			ubo.bones[i] = glm::transpose(glm::make_mat4(&(boneTransforms[i].a1)));
		}
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation Key-frames");
	}
}