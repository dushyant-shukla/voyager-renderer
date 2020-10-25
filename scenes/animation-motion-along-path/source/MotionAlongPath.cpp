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
			for (auto& model : mModels)
			{
				delete model;
			}

			for (size_t i = 0; i < mSwapchain->mImages.size(); i++)
			{
				vkDestroyBuffer(LOGICAL_DEVICE, viewUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, viewUboMemory[i], nullptr);
				vkDestroyBuffer(LOGICAL_DEVICE, skinningUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, skinningUboMemory[i], nullptr);
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
		SetupTextureSamplers();
		SetupDescriptorSet();
		SetupPipeline();

		LoadAssets();

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

	void MotionAlongPath::SetupDescriptorSet()
	{
		SetupUbo();

		// mesh descriptor sets
		{
			mDescriptorPools.mesh
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.Create(0, static_cast<unsigned int>(mSwapchain->mImages.size()), nullptr);

			mDescriptorSetLayouts.mesh
				.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
				.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
				.Create(0, nullptr);

			mDescriptorSets.mesh.Setup(mDescriptorSetLayouts.mesh.mLayout,
				mDescriptorPools.mesh.mPool,
				static_cast<unsigned int>(mSwapchain->mImages.size()));

			std::vector<VkWriteDescriptorSet> descriptorWrites = {};
			for (size_t i = 0; i < mDescriptorSets.mesh.mSets.size(); ++i)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = viewUboBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(viewUBO);

				VkWriteDescriptorSet writeBufferInfo = {};
				writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeBufferInfo.dstSet = mDescriptorSets.mesh.mSets[i];
				writeBufferInfo.dstBinding = 0;
				writeBufferInfo.dstArrayElement = 0;
				writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeBufferInfo.descriptorCount = 1;
				writeBufferInfo.pBufferInfo = &bufferInfo;

				descriptorWrites.push_back(writeBufferInfo);
			}
			vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

			descriptorWrites = {};
			for (size_t i = 0; i < mDescriptorSets.mesh.mSets.size(); ++i)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = skinningUboBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(skinningUBO);

				VkWriteDescriptorSet writeBufferInfo = {};
				writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeBufferInfo.dstSet = mDescriptorSets.mesh.mSets[i];
				writeBufferInfo.dstBinding = 1;
				writeBufferInfo.dstArrayElement = 0;
				writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeBufferInfo.descriptorCount = 1;
				writeBufferInfo.pBufferInfo = &bufferInfo;

				descriptorWrites.push_back(writeBufferInfo);
			}
			vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		// curve descriptor sets
		{
			mDescriptorPools.curve
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.Create(0, static_cast<unsigned int>(mSwapchain->mImages.size()), nullptr);

			mDescriptorSetLayouts.curve
				.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
				.Create(0, nullptr);

			mDescriptorSets.curve.Setup(mDescriptorSetLayouts.curve.mLayout,
				mDescriptorPools.curve.mPool,
				static_cast<unsigned int>(mSwapchain->mImages.size()));

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

				descriptorWrites.push_back(writeBufferInfo);
			}
			vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		// texture sampler descriptor layout and pool
		{
			mDescriptorPools.texture
				.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
				.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
				.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
				.Create(0, MAX_MESH_COUNT, nullptr); // each mesh will own a descriptor set for holding texture data

			mDescriptorSetLayouts.texture
				.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler diffuse
				.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler specular
				.AddLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler emission
				.Create(0, nullptr);
		}
	}

	void MotionAlongPath::SetupPipeline()
	{
		// pipeline for mesh
		{
			mPipelineLayouts.mesh
				.AddDescriptorSetLayout(mDescriptorSetLayouts.mesh.mLayout)
				.AddDescriptorSetLayout(mDescriptorSetLayouts.texture.mLayout)
				.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelData))
				.Configure();

			mPipelines.mesh
				.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "motion-along-path/vertex-skinning.vert.spv")
				.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "motion-along-path/vertex-skinning.frag.spv")
				.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
				.AddVertexInputBindingDescription(vrassimp::MeshVertex::GetVertexInputBindingDescription())
				.AddVertexInputAttributeDescription(vrassimp::MeshVertex::GetVertexInputAttributeDescriptions())
				.ConfigureViewport(mSwapchain->mExtent)
				.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
				.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
				.ConfigureDefaultDepthTesting()
				.AddColorBlendAttachmentState(VK_TRUE,
					VK_BLEND_FACTOR_SRC_ALPHA,
					VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					VK_BLEND_OP_ADD,
					VK_BLEND_FACTOR_SRC_ALPHA,
					VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					VK_BLEND_OP_SUBTRACT,
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
				.ConfigureColorBlendState(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
				.Configure(mPipelineLayouts.mesh.GetVulkanPipelineLayout(), mRenderpass.mRenderPass, 0, 0);
		}

		// pipeline for curve
		{
			mPipelineLayouts.curve
				.AddDescriptorSetLayout(mDescriptorSetLayouts.curve.mLayout)
				.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(primitiveModelData))
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
	}

	void MotionAlongPath::SetupUbo()
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

		bufferSize = sizeof(skinningUBO);
		skinningUboBuffers.resize(mSwapchain->mImages.size());
		skinningUboMemory.resize(mSwapchain->mImages.size());
		for (unsigned int i = 0; i < mSwapchain->mImages.size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ALLOCATION_CALLBACK, &skinningUboBuffers[i], &skinningUboMemory[i]);
		}
	}

	void MotionAlongPath::SetupTextureSamplers()
	{
		mSamplers.diffuse.CreateDefault();
		mSamplers.specular.CreateDefault();
		mSamplers.emission.CreateDefault();
	}

	void MotionAlongPath::LoadAssets()
	{
		vrassimp::Model* floor = new vrassimp::Model();
		floor->LoadFromFile("floor\\scene.gltf", "floor");
		floor->mTransform.position = glm::vec3(20.0f, 0.0f, 0.0f);
		floor->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		floor->mTransform.scale = glm::vec3(30.0f, 1.0f, 30.0f);
		mModels.push_back(floor);

		vrassimp::Model* nathan = new vrassimp::Model();
		nathan->LoadFromFile("nathan\\scene.gltf", "nathan");
		nathan->mTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		nathan->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		nathan->mTransform.scale = glm::vec3(0.05, 0.05f, 0.05f);
		nathan->isAnimationAvailable = false;
		mModels.push_back(nathan);

		for (auto& model : mModels)
		{
			for (auto& mesh : model->meshes)
			{
				mesh->buffers.vertex.Create(mesh->vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				mesh->buffers.index.Create(mesh->indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

				if (!mesh->textures.empty())
				{
					mesh->mDescriptorSets.Setup(mDescriptorSetLayouts.texture.mLayout, mDescriptorPools.texture.mPool, 1);
					std::vector<VkWriteDescriptorSet> descriptorWrites;

					for (auto texture : mesh->textures)
					{
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
							writeImageInfo.dstSet = mesh->mDescriptorSets[0];
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

	void MotionAlongPath::UpdateUniformBuffer(unsigned int imageIndex)
	{
		// view ubo
		//viewUBO.projection = eCamera.matrices.projection;
		//viewUBO.view = eCamera.matrices.view;

		viewUBO.projection = mCamera.project;
		viewUBO.view = mCamera.view;

		void* data;
		vkMapMemory(LOGICAL_DEVICE, viewUboMemory[imageIndex], 0, sizeof(viewUBO), 0, &data);
		memcpy(data, &viewUBO, sizeof(viewUBO));
		vkUnmapMemory(LOGICAL_DEVICE, viewUboMemory[imageIndex]);

		// skinning ubo
	}

	void MotionAlongPath::UpdateModelData(vrassimp::Model* model)
	{
		glm::mat4  modelMatrix(1.0);
		modelMatrix = glm::translate(modelMatrix, model->mTransform.position);
		modelMatrix = glm::rotate(modelMatrix, model->mTransform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, model->mTransform.rotation.y, glm::vec3(0.0f, 0.0f, 1.0f)); // y is rotated around z-axis
		modelMatrix = glm::rotate(modelMatrix, model->mTransform.rotation.z, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, model->mTransform.scale);
		modelData.model = modelMatrix;

		if (model->isAnimationAvailable)
		{
			// if model has animation, take current state from UI's animation setting
			modelData.enableAnimation = animationSettings.enableAnimation;
		}
		else
		{
			modelData.enableAnimation = 0;
		}
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

		UpdateUniformBuffer(imageIndex);

		// draw curve
		{
			vkCmdBindPipeline(mGraphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.curve.GetVulkanPipeline());
			std::vector<VkDescriptorSet> descriptorSets = { mDescriptorSets.curve.mSets[imageIndex] };
			vkCmdBindDescriptorSets(mGraphicsCommandBuffers[imageIndex],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				mPipelineLayouts.curve.mLayout,
				0,
				static_cast<unsigned int>(descriptorSets.size()),
				descriptorSets.data(), 0, nullptr);

			primitiveModelData.isControlPoints = false;
			vkCmdPushConstants(mGraphicsCommandBuffers[imageIndex],
				mPipelineLayouts.curve.mLayout,
				VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(primitiveModelData), &primitiveModelData);
			curve->DrawSpline(mGraphicsCommandBuffers[imageIndex]);

			primitiveModelData.isControlPoints = true;
			vkCmdPushConstants(mGraphicsCommandBuffers[imageIndex],
				mPipelineLayouts.curve.mLayout,
				VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(primitiveModelData), &primitiveModelData);
			curve->DrawControlPoints(mGraphicsCommandBuffers[imageIndex]);
		}

		// draw models
		{
			for (auto& model : mModels)
			{
				UpdateModelData(model);

				vkCmdBindPipeline(mGraphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.mesh.GetVulkanPipeline());

				vkCmdPushConstants(mGraphicsCommandBuffers[imageIndex],
					mPipelineLayouts.mesh.GetVulkanPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT, 0,
					sizeof(modelData), &modelData);	// we can also pass an array of data

				for (auto& mesh : model->meshes)
				{
					std::vector<VkDescriptorSet> descriptorSets = { mDescriptorSets.mesh.mSets[imageIndex] };
					if (!mesh->textures.empty())
					{
						descriptorSets.push_back(mesh->mDescriptorSets.mSets[0]);
					}

					vkCmdBindDescriptorSets(mGraphicsCommandBuffers[imageIndex],
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						mPipelineLayouts.mesh.GetVulkanPipelineLayout(),
						0,
						static_cast<unsigned int>(descriptorSets.size()),
						descriptorSets.data(), 0, nullptr);

					mesh->Draw(mGraphicsCommandBuffers[imageIndex]);
				}
			}
		}

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