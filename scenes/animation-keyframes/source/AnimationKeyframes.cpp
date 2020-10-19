#include "AnimationKeyframes.h"
#include "RendererState.h"
#include "window/Window.h"

#include <imgui.h>

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name)
	{
		eCamera.type = EditingModeCamera::Type::look_at;
		eCamera.matrices.flipY = true;
		eCamera.SetPosition(glm::vec3(0.0f, 0.75f, -2.0f));
		eCamera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		eCamera.SetPerspective(glm::radians(60.0f), (float)Window::WIDTH / Window::HEIGHT, 0.1f, 1024.0f);
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
				vkDestroyBuffer(LOGICAL_DEVICE, meshUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, meshUboMemory[i], nullptr);

				vkDestroyBuffer(LOGICAL_DEVICE, primitiveUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, primitiveUboMemory[i], nullptr);
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

		meshUBO.projection = eCamera.matrices.projection;
		meshUBO.view = eCamera.matrices.view;
		meshUBO.viewPosition = eCamera.orientation.viewPosition;
		UpdateBoneTransforms();

		// NATHAN
		//glm::mat4 model(1.0f);/* = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));*/
		//model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
		//model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		////model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.50f)); // tiger
		//model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); // nathan
		////model = glm::scale(model, glm::vec3(3.00f, 3.0f, 3.0f)); // spidey
		//mPerModelData.model = model;

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

		timer += frametime * animationSettings.speed;
	}

	VkPhysicalDeviceFeatures AnimationKeyframes::CheckRequiredFeatures()
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

	void AnimationKeyframes::SetupPipeline()
	{
		// pipeline layout
		//								1. descriptor layout
		//								2. push constant for model matrix
		mPipelineLayouts.mesh
			.AddDescriptorSetLayout(mDescriptors.meshUboLayout.GetVkDescriptorSetLayout())
			.AddDescriptorSetLayout(mDescriptors.textureLayout.mLayout)
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPerModelData))
			.Configure();

		// setup pipeline
		mPipelines.mesh // TODO setup shaders
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "vertex-skinning/vertex-skinning.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "vertex-skinning/vertex-skinning.frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(vrassimp::MeshVertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(vrassimp::MeshVertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->GetSwapchainExtent())
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

		mPipelineLayouts.joints
			.AddDescriptorSetLayout(mDescriptors.primitiveUboLayout.mLayout)
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPrimtiveModelData))
			.Configure();

		mPipelines.joints
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "vertex-skinning/primitive-drawing.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "vertex-skinning/primitive-drawing.frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(vrassimp::JointVertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(vrassimp::JointVertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->GetSwapchainExtent())
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 5.0f, 0, nullptr)
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
			.Configure(mPipelineLayouts.joints.mLayout, mRenderpass.mRenderPass, 0, 0);

		mPipelineLayouts.bones
			.AddDescriptorSetLayout(mDescriptors.primitiveUboLayout.mLayout)
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPrimtiveModelData))
			.Configure();

		mPipelines.bones
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "vertex-skinning/primitive-drawing.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "vertex-skinning/primitive-drawing.frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(vrassimp::JointVertex::GetVertexInputBindingDescription())
			.AddVertexInputAttributeDescription(vrassimp::JointVertex::GetVertexInputAttributeDescriptions())
			.ConfigureViewport(mSwapchain->GetSwapchainExtent())
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 2.0f, 0, nullptr)
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
			.Configure(mPipelineLayouts.joints.mLayout, mRenderpass.mRenderPass, 0, 0);
	}

	void AnimationKeyframes::RecordCommands(const unsigned int& currentImage)
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
		renderpassBeginInfo.framebuffer = mFramebuffers[currentImage];

		// begin command buffer
		CHECK_RESULT(vkBeginCommandBuffer(mGraphicsCommandBuffers[currentImage], &bufferBeginInfo), "FAILED TO START RECORDING A COMMAND BUFFER!");

		// being render pass
		vkCmdBeginRenderPass(mGraphicsCommandBuffers[currentImage], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (size_t i = 0; i < mModels.size(); ++i)
		{
			vrassimp::Model* currentModel = mModels[i];
			UpdateModelInfo(currentModel);

			if (!animationSettings.boneLine)
			{
				// bind pipeline to be used in render pass
				vkCmdBindPipeline(mGraphicsCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.mesh.GetVulkanPipeline());

				// all meshes within a model will have the same model matrix
				vkCmdPushConstants(mGraphicsCommandBuffers[currentImage],
					mPipelineLayouts.mesh.GetVulkanPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT, 0,
					sizeof(mPerModelData), &mPerModelData);	// we can also pass an array of data

				for (size_t j = 0; j < currentModel->meshes.size(); ++j)
				{
					vrassimp::Mesh* currentMesh = currentModel->meshes[j];

					std::vector<VkDescriptorSet> descriptorSets = { mDescriptors.meshUboSets[currentImage] }; //, currentMesh->mDescriptorSets[0] };

					if (!currentMesh->textures.empty())
					{
						descriptorSets.push_back(currentMesh->mDescriptorSets[0]);
					}

					vkCmdBindDescriptorSets(mGraphicsCommandBuffers[currentImage],
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						mPipelineLayouts.mesh.GetVulkanPipelineLayout(),
						0,
						static_cast<unsigned int>(descriptorSets.size()),
						descriptorSets.data(), 0, nullptr);

					//vrassimp::Mesh* currentMesh = currentModel->meshes[j];
					currentMesh->Draw(mGraphicsCommandBuffers[currentImage]); // bind vertex and index buffer, cmdIndexedDraw()
				}
			}
			else // bones are enabled
			{
				// render joints
				{
					vkCmdBindPipeline(mGraphicsCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.joints.GetVulkanPipeline());

					glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
					//model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); // nathan
					//model = glm::scale(model, glm::vec3(0.50f, 0.50f, 0.50f)); // tiger
					model = glm::scale(model, glm::vec3(3.00f, 3.00f, 3.00f)); // spidey
					mPrimtiveModelData.model = model;

					// all meshes within a model will have the same model matrix
					vkCmdPushConstants(mGraphicsCommandBuffers[currentImage],
						mPipelineLayouts.joints.GetVulkanPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT, 0,
						sizeof(mPrimtiveModelData), &mPrimtiveModelData);	// we can also pass an array of data

					std::vector<VkDescriptorSet> descriptorSets = { mDescriptors.primitiveUboSets[currentImage] };

					vkCmdBindDescriptorSets(mGraphicsCommandBuffers[currentImage],
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						mPipelineLayouts.joints.GetVulkanPipelineLayout(),
						0,
						static_cast<unsigned int>(descriptorSets.size()),
						descriptorSets.data(), 0, nullptr);

					// bind vertices
					jointVertexBuffer.Unmap();
					vkQueueWaitIdle(GRAPHICS_QUEUE);
					jointVertexBuffer.Destroy();
					MemoryUtility::CreateBuffer(sizeof(currentModel->jointPositions[0]) * currentModel->mAnimation->mBoneCount,
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
						ALLOCATION_CALLBACK,
						&jointVertexBuffer.mBuffer,
						&jointVertexBuffer.mMemory);

					jointVertexBuffer.Map();
					jointVertexBuffer.CopyData(&currentModel->jointPositions[0], sizeof(currentModel->jointPositions[0]) * currentModel->mAnimation->mBoneCount);

					VkBuffer vertexBuffers[] = { jointVertexBuffer.mBuffer };	// buffers to bind
					VkDeviceSize offsets[] = { 0 };										// offsets into buffers being bound
					vkCmdBindVertexBuffers(mGraphicsCommandBuffers[currentImage], 0, 1, vertexBuffers, offsets);

					vkCmdDraw(mGraphicsCommandBuffers[currentImage], currentModel->mAnimation->mBoneCount, 1, 0, 0);

					jointVertexBuffer.Unmap();
				}

				// render bones
				{
					vkCmdBindPipeline(mGraphicsCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.bones.GetVulkanPipeline());

					glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
					//model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); // nathan
					//model = glm::scale(model, glm::vec3(0.50f, 0.50f, 0.50f)); // tiger
					model = glm::scale(model, glm::vec3(3.00f, 3.00f, 3.00f)); // spidey
					mPrimtiveModelData.model = model;

					// all meshes within a model will have the same model matrix
					vkCmdPushConstants(mGraphicsCommandBuffers[currentImage],
						mPipelineLayouts.joints.GetVulkanPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT, 0,
						sizeof(mPrimtiveModelData), &mPrimtiveModelData);	// we can also pass an array of data

					std::vector<VkDescriptorSet> descriptorSets = { mDescriptors.primitiveUboSets[currentImage] };

					vkCmdBindDescriptorSets(mGraphicsCommandBuffers[currentImage],
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						mPipelineLayouts.bones.GetVulkanPipelineLayout(),
						0,
						static_cast<unsigned int>(descriptorSets.size()),
						descriptorSets.data(), 0, nullptr);

					boneVertexBuffer.Unmap();
					vkQueueWaitIdle(GRAPHICS_QUEUE);
					boneVertexBuffer.Destroy();
					MemoryUtility::CreateBuffer(sizeof(currentModel->linePositions[0]) * currentModel->mAnimation->mBoneCount * 2,
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
						ALLOCATION_CALLBACK,
						&boneVertexBuffer.mBuffer,
						&boneVertexBuffer.mMemory);

					boneVertexBuffer.Map();
					boneVertexBuffer.CopyData(&currentModel->linePositions[0], sizeof(currentModel->linePositions[0]) * currentModel->mAnimation->mBoneCount * 2);

					VkBuffer vertexBuffers[] = { boneVertexBuffer.mBuffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(mGraphicsCommandBuffers[currentImage], 0, 1, vertexBuffers, offsets);

					vkCmdDraw(mGraphicsCommandBuffers[currentImage], currentModel->mAnimation->mBoneCount * 2, 1, 0, 0);

					boneVertexBuffer.Unmap();
				}
			}
		}

		DrawUI(mGraphicsCommandBuffers[currentImage]);

		// end render pass
		vkCmdEndRenderPass(mGraphicsCommandBuffers[currentImage]);

		if (vkEndCommandBuffer(mGraphicsCommandBuffers[currentImage]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop recording a command buffer!!");
		}
	}

	void AnimationKeyframes::SetupDescriptorSets()
	{
		mDescriptors.meshUboPool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.Create(0, static_cast<unsigned> (mSwapchain->GetSwapchainImages().size()), nullptr);

		mDescriptors.meshUboLayout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)				// view projection ubo
			.Create(0, nullptr);

		// Allocating descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(mSwapchain->GetSwapchainImages().size(), mDescriptors.meshUboLayout.GetVkDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptors.meshUboPool.GetVulkanDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->GetSwapchainImages().size());
		allocInfo.pSetLayouts = layouts.data();
		mDescriptors.meshUboSets.resize(mSwapchain->GetSwapchainImages().size());
		CHECK_RESULT(vkAllocateDescriptorSets(mDevice->GetLogicalDevice().device, &allocInfo, mDescriptors.meshUboSets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SETS");

		// configuring allocated descriptor sets with buffer and image information
		std::vector<VkWriteDescriptorSet> descriptorWrites = {};
		for (size_t i = 0; i < mDescriptors.meshUboSets.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = meshUboBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(meshUBO);

			VkWriteDescriptorSet writeBufferInfo = {};
			writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeBufferInfo.dstSet = mDescriptors.meshUboSets[i];
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

		/*
			Primitive draw descriptor configuration
		*/
		mDescriptors.primitiveUboPool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.Create(0, static_cast<unsigned> (mSwapchain->GetSwapchainImages().size()), nullptr);

		mDescriptors.primitiveUboLayout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
			.Create(0, nullptr);

		// Allocating descriptor sets
		std::vector<VkDescriptorSetLayout> primitiveLayouts(mSwapchain->GetSwapchainImages().size(), mDescriptors.primitiveUboLayout.mLayout);
		VkDescriptorSetAllocateInfo primitiveDesAllocInfo{};
		primitiveDesAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		primitiveDesAllocInfo.descriptorPool = mDescriptors.primitiveUboPool.mPool;
		primitiveDesAllocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->GetSwapchainImages().size());
		primitiveDesAllocInfo.pSetLayouts = primitiveLayouts.data();
		mDescriptors.primitiveUboSets.resize(mSwapchain->GetSwapchainImages().size());
		CHECK_RESULT(vkAllocateDescriptorSets(LOGICAL_DEVICE, &primitiveDesAllocInfo, mDescriptors.primitiveUboSets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SETS");

		// configuring allocated descriptor sets with buffer and image information
		descriptorWrites = {};
		for (size_t i = 0; i < mDescriptors.primitiveUboSets.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = primitiveUboBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(primitiveUBO);

			VkWriteDescriptorSet writeBufferInfo = {};
			writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeBufferInfo.dstSet = mDescriptors.primitiveUboSets[i];
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

	void AnimationKeyframes::SetupUniformBufferObjects()
	{
		VkDeviceSize bufferSize = sizeof(meshUBO);
		meshUboBuffers.resize(mSwapchain->GetSwapchainImages().size());
		meshUboMemory.resize(mSwapchain->GetSwapchainImages().size());
		for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				nullptr, &meshUboBuffers[i], &meshUboMemory[i]);
		}

		bufferSize = sizeof(primitiveUBO);
		primitiveUboBuffers.resize(mSwapchain->GetSwapchainImages().size());
		primitiveUboMemory.resize(mSwapchain->GetSwapchainImages().size());
		for (size_t i = 0; i < mSwapchain->GetSwapchainImages().size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				nullptr, &primitiveUboBuffers[i], &primitiveUboMemory[i]);
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
		//model->LoadFromFile("nathan\\scene.gltf"); // works - loads correctly
		//model->LoadFromFile("bengal-tiger\\tiger.fbx"); // works - loads correctly
		//model->LoadFromFile("myth-creature\\myth-creature.fbx", &modelCreateInfo); // works - loads correctly
		model->LoadFromFile("spiderman\\spiderman.fbx"); // works - loads correctly

		// TODO: read archetype file here for Nathan which will hold the name for model file, and transformation information
		// mimic the behavior for now
		model->mTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		model->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		//model->mTransform.scale = glm::vec3(0.01f, 0.01f, 0.01f); // nathan
		//model->mTransform.scale = glm::vec3(0.5f, 0.5f, 0.50f);   // tiger
		model->mTransform.scale = glm::vec3(3.00f, 3.0f, 3.0f);   // spidey

		for (size_t i = 0; i < model->mAnimation->animationTimes.size(); ++i)
		{
			animationSettings.animations += (model->mAnimation->animationTimes[i].name + '\0');
		}
		animationSettings.animations += '\0';

		/*
			Setup meshes' vertex buffers, index buffers and textures
		*/

		for (size_t meshIndex = 0; meshIndex < model->meshes.size(); ++meshIndex)
		{
			vrassimp::Mesh* currentMesh = model->meshes[meshIndex];

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
					vrassimp::Texture* texture = model->meshes[meshIndex]->textures[textureIndex];
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

		mModels.push_back(model);
	}

	void AnimationKeyframes::UpdateUniformBuffers(const unsigned int& imageIndex)
	{
		meshUBO.projection = eCamera.matrices.projection;
		meshUBO.view = eCamera.matrices.view;
		meshUBO.viewPosition = eCamera.orientation.viewPosition;
		UpdateBoneTransforms();

		void* data;
		vkMapMemory(LOGICAL_DEVICE, meshUboMemory[imageIndex], 0, sizeof(meshUBO), 0, &data);
		memcpy(data, &meshUBO, sizeof(meshUBO));
		vkUnmapMemory(LOGICAL_DEVICE, meshUboMemory[imageIndex]);

		primitiveUBO.projection = eCamera.matrices.projection;
		primitiveUBO.view = eCamera.matrices.view;
		void* data1;
		vkMapMemory(LOGICAL_DEVICE, primitiveUboMemory[imageIndex], 0, sizeof(primitiveUBO), 0, &data1);
		memcpy(data1, &primitiveUBO, sizeof(primitiveUBO));
		vkUnmapMemory(LOGICAL_DEVICE, primitiveUboMemory[imageIndex]);
	}

	void AnimationKeyframes::UpdateModelInfo(vrassimp::Model* model)
	{
		glm::mat4 modelMatrix(1.0f);
		modelMatrix = glm::scale(modelMatrix, model->mTransform.scale);
		modelMatrix = glm::rotate(modelMatrix, model->mTransform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, model->mTransform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, model->mTransform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::translate(modelMatrix, model->mTransform.position);
		mPerModelData.model = modelMatrix;
	}

	glm::mat4 AnimationKeyframes::aiMatrix4x4ToGlm(aiMatrix4x4 ai_matr)
	{
		glm::mat4 result;
		result[0].x = ai_matr.a1; result[0].y = ai_matr.b1; result[0].z = ai_matr.c1; result[0].w = ai_matr.d1;
		result[1].x = ai_matr.a2; result[1].y = ai_matr.b2; result[1].z = ai_matr.c2; result[1].w = ai_matr.d2;
		result[2].x = ai_matr.a3; result[2].y = ai_matr.b3; result[2].z = ai_matr.c3; result[2].w = ai_matr.d3;
		result[3].x = ai_matr.a4; result[3].y = ai_matr.b4; result[3].z = ai_matr.c4; result[3].w = ai_matr.d4;

		return result;
	}

	void AnimationKeyframes::UpdateBoneTransforms()
	{
		for (unsigned int i = 0; i < mModels.size(); ++i)
		{
			vrassimp::Model* currentModel = mModels[i];

			vrassimp::Animation* animation = currentModel->mAnimation;
			animation->currentIndex = animationSettings.id;
			if (timer > animation->animationTimes[animation->currentIndex].end)
			{
				//animation->currentIndex = (animation->currentIndex + 1) % animation->animationTimes.size();
				RENDERER_INFO("CURRENT ANIMATION: {0}", animation->currentIndex);
				timer = animation->animationTimes[animation->currentIndex].start;
			}

			std::vector<aiMatrix4x4> transforms;
			std::vector<aiMatrix4x4> boneTransforms; // for drawing bones without mesh
			currentModel->mAnimation->Animate(timer, transforms, boneTransforms);
			for (unsigned int j = 0; j < transforms.size(); ++j)
			{
				meshUBO.bones[j] = glm::transpose(glm::make_mat4(&(transforms[j].a1)));
			}

			// joints positions to render
			currentModel->jointPositions.clear();
			currentModel->jointPositions.resize(currentModel->mAnimation->mBoneCount); // mBoneCount is same as boneTransform.size()
			for (unsigned int j = 0; j < boneTransforms.size(); ++j)
			{
				glm::mat4 t = glm::transpose(glm::make_mat4(&(boneTransforms[j].a1)));
				glm::vec4 v = t * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				currentModel->jointPositions[j] = { v.x, v.y, v.z, 1 };
			}

			// bone line positions to render
			currentModel->linePositions = {};
			currentModel->linePositions.resize(currentModel->mAnimation->mBoneCount * 2); // mBoneCount is same as boneTransform.size()
			std::vector<glm::vec4> BonePositionsMeshSpaceChild;
			std::vector<glm::vec4> BonePositionsMeshSpaceParent;

			for (unsigned int j = 1; j < currentModel->mAnimation->mBoneCount; ++j)
			{
				unsigned int k = j * 2;
				glm::mat4 childPos = glm::transpose(glm::make_mat4(&(currentModel->mAnimation->boneEndpointPositions[j].childBone.a1)));
				glm::mat4 parentPos = glm::transpose(glm::make_mat4(&(currentModel->mAnimation->boneEndpointPositions[j].parentBone.a1)));

				BonePositionsMeshSpaceChild.push_back(childPos * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				BonePositionsMeshSpaceParent.push_back(parentPos * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

				currentModel->linePositions[k] = { BonePositionsMeshSpaceChild[j - 1].x, BonePositionsMeshSpaceChild[j - 1].y, BonePositionsMeshSpaceChild[j - 1].z, 1 };
				currentModel->linePositions[k + 1] = { BonePositionsMeshSpaceParent[j - 1].x, BonePositionsMeshSpaceParent[j - 1].y, BonePositionsMeshSpaceParent[j - 1].z, 1 };
			}
		}
	}

	void AnimationKeyframes::OnUpdateUIOverlay(UiOverlay* overlay)
	{
		ImGui::SetNextWindowPos(ImVec2(10, 200));
		ImGui::Begin("Animation (Key-Frames)");
		ImGui::Text("Settings:");
		if (overlay->CheckBox("Enable animation", &mPerModelData.enableAnimation))
		{
			int a = 10;
		}
		if (overlay->CheckBox("Enable bones", &animationSettings.boneLine))
		{
		}
		if (mPerModelData.enableAnimation && overlay->InputFloat("speed", &animationSettings.speed, 0.5, 3))
		{
		}
		if (mPerModelData.enableAnimation && ImGui::Combo("track", &animationSettings.id, animationSettings.animations.c_str()))
		{
		}
		ImGui::End();
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation Key-frames");
	}
}