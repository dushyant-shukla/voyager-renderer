#include "ClothSimulation.h"
#include "input/InputManager.h"

#include <imgui.h>

namespace vr
{
	ClothSimulation::ClothSimulation(std::string name) : Application(name)
	{
		activeCamera = CameraType::FIRST_PERSON;
		cameraTypeIndex = static_cast<int>(activeCamera);

		mCamera.position = glm::vec3(0.0f, 5.00f, 15.0f);
		mCamera.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

		eCamera.type = EditingModeCamera::Type::look_at;
		eCamera.matrices.flipY = true;
		eCamera.SetPosition(glm::vec3(0.0f, 5.00f, -20.0f));
		eCamera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		eCamera.SetPerspective(glm::radians(60.0f), (float)Window::WIDTH / Window::HEIGHT, 0.1f, 1024.0f);

		lightUBO.color = glm::vec3(1.0f, 1.0f, 1.0f);
		lightUBO.position = glm::vec3(0.0f, 10.0f, 5.0f);
		//lightUBO.direction = glm::vec3(0.0f, 1.0f, 0.0f);
		//lightUBO.cutoff = glm::cos(glm::radians(12.5f));
		lightUBO.constant = 1.0f;
		lightUBO.linear = 0.22f;
		lightUBO.quadratic = 0.20f;
	}

	ClothSimulation::~ClothSimulation()
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
				vkDestroyBuffer(LOGICAL_DEVICE, lightUboBuffers[i], nullptr);
				vkFreeMemory(LOGICAL_DEVICE, lightUboMemory[i], nullptr);
			}
			RENDERER_DEBUG("RESOURCE DESTROYED: UNIFORM BUFFER");
			RENDERER_DEBUG("RESOURCE FREED: UNIFORM BUFFER MEMORY");
		}
	}

	void ClothSimulation::InitializeScene()
	{
		SetupTextureSamplers();
		SetupDescriptorSet();
		SetupPipeline();
		LoadAssets();

		mCloth = Cloth(1, 1, 50, 50);
		//VkDeviceSize size = 115248 * sizeof(float);
		VkDeviceSize size = 14406 * sizeof(ClothVertex);
		MemoryUtility::CreateBuffer(size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			ALLOCATION_CALLBACK,
			&mClothBuffer.mBuffer,
			&mClothBuffer.mMemory);
		mClothBuffer.Map();

		clothModel.position = glm::vec3(0.0f, 0.0f, 0.0f);
		clothModel.rotation = glm::vec3(0.0f);
		clothModel.scale = glm::vec3(1.0f, 1.0f, 1.0f);

		isReady = true;
	}

	void ClothSimulation::CleanupScene()
	{
	}

	void ClothSimulation::Draw(const double& frametime)
	{
		vkWaitForFences(LOGICAL_DEVICE, 1, &mSyncPrimitives.mDrawFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(LOGICAL_DEVICE, 1, &mSyncPrimitives.mDrawFences[mCurrentFrame]);

		unsigned int imageIndex;
		vkAcquireNextImageKHR(LOGICAL_DEVICE, mSwapchain->mSwapchain, std::numeric_limits<uint64_t>::max(), mSyncPrimitives.mImageAvailable[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

		RecordCommands(imageIndex, frametime);

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

		// Get next frame (% keeps value below MAX_FRAME_DRAWS)
		mCurrentFrame = (mCurrentFrame + 1) % SynchronizationPrimitives::MAX_FRAME_DRAWS;
	}

	VkPhysicalDeviceFeatures ClothSimulation::CheckRequiredFeatures()
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

	void ClothSimulation::OnUpdateUIOverlay(UiOverlay* overlay)
	{
		ImGui::SetNextWindowPos(ImVec2(10, 210));
		ImGui::Begin("Cloth Simulation");
		ImGui::Text("Settings:");
		if (ImGui::TreeNode("Light"))
		{
			ImGui::Text("Position: ");
			//ImGui::SameLine();
			ImGui::PushItemWidth(100);
			ImGui::InputFloat("x", &(lightUBO.position.x), 0.05f, 1.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(100);
			ImGui::InputFloat("y", &(lightUBO.position.y), 0.05f, 1.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(100);
			ImGui::InputFloat("z", &(lightUBO.position.z), 0.05f, 1.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::Text("Color: ");
			ImGui::ColorEdit3("##light_color##2f", (float*)&lightUBO.color, ImGuiColorEditFlags_Float);

			ImGui::Text("Attenuation: ");
			ImGui::SliderFloat("constant", &(lightUBO.constant), 0.01f, 2.0, "%.2f");
			ImGui::SliderFloat("linear", &(lightUBO.linear), 0.0001f, 1.0, "%.4f");
			ImGui::SliderFloat("quadratic", &(lightUBO.quadratic), 0.000001f, 2.0, "%.6f");

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Cloth"))
		{
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##scale_x", &(clothModel.scale.x), 0.01f, -9999.0, 9999.0, "S:x = %.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##scale_y", &(clothModel.scale.y), 0.01f, -9999.0, 9999.0, "S:y = %.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##scale_z", &(clothModel.scale.z), 0.01f, -9999.0, 9999.0, "S:z = %.2f");
			ImGui::PopItemWidth();

			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##rotate_x", &(clothModel.rotation.x), 0.05f, -9999.0, 9999.0, "R:x = %.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##rotate_y", &(clothModel.rotation.y), 0.05f, -9999.0, 9999.0, "R:y = %.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##rotate_z", &(clothModel.rotation.z), 0.05f, -9999.0, 9999.0, "R:z = %.2f");
			ImGui::PopItemWidth();

			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##translate_x", &(clothModel.position.x), 0.05f, -9999.0, 9999.0, "T:x = %.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##translate_y", &(clothModel.position.y), 0.05f, -9999.0, 9999.0, "T:y = %.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("##translate_z", &(clothModel.position.z), 0.05f, -9999.0, 9999.0, "T:z = %.2f");
			ImGui::PopItemWidth();

			if (overlay->CheckBox("simulate wind", &(mSettings.wind)))
			{
			}
			ImGui::TreePop();
		}

		std::string modelText = "Models (";
		modelText += std::to_string(mModels.size());
		modelText += ")";
		if (ImGui::TreeNode(modelText.c_str()))
		{
			for (auto& model : mModels)
			{
				bool showStaticTransforms = true;
				if (ImGui::TreeNode(model->mScreenName.c_str()))
				{
					if (model->mScreenName._Equal("football"))
					{
						ImGui::DragFloat("##radius", &(ballRadius), 0.01f, 0.01f, 50.0f, "Radius = %.2f");
						model->mTransform.scale.x = ballRadius;
						model->mTransform.scale.y = ballRadius;
						model->mTransform.scale.z = ballRadius;
					}
					else
					{
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##scale_x", &(model->mTransform.scale.x), 0.01f, -9999.0, 9999.0, "S:x = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##scale_y", &(model->mTransform.scale.y), 0.01f, -9999.0, 9999.0, "S:y = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##scale_z", &(model->mTransform.scale.z), 0.01f, -9999.0, 9999.0, "S:z = %.2f");
						ImGui::PopItemWidth();
					}

					ImGui::PushItemWidth(80);
					ImGui::DragFloat("##rotate_x", &(model->mTransform.rotation.x), 0.05f, -9999.0, 9999.0, "R:x = %.2f");
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ImGui::PushItemWidth(80);
					ImGui::DragFloat("##rotate_y", &(model->mTransform.rotation.y), 0.05f, -9999.0, 9999.0, "R:y = %.2f");
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ImGui::PushItemWidth(80);
					ImGui::DragFloat("##rotate_z", &(model->mTransform.rotation.z), 0.05f, -9999.0, 9999.0, "R:z = %.2f");
					ImGui::PopItemWidth();

					ImGui::PushItemWidth(80);
					ImGui::DragFloat("##translate_x", &(model->mTransform.position.x), 0.05f, -9999.0, 9999.0, "T:x = %.2f");
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ImGui::PushItemWidth(80);
					ImGui::DragFloat("##translate_y", &(model->mTransform.position.y), 0.05f, -9999.0, 9999.0, "T:y = %.2f");
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ImGui::PushItemWidth(80);
					ImGui::DragFloat("##translate_z", &(model->mTransform.position.z), 0.05f, -9999.0, 9999.0, "T:z = %.2f");
					ImGui::PopItemWidth();

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
		ImGui::End();
	}

	void ClothSimulation::SetupDescriptorSet()
	{
		SetupUbo();

		// mesh descriptor set
		{
			mDescriptorPools.mesh
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.Create(0, static_cast<unsigned int>(mSwapchain->mImages.size()), nullptr);

			mDescriptorSetLayouts.mesh
				.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
				.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)
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
				bufferInfo.buffer = lightUboBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(lightUBO);

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

		// texture sampler descriptor layout and pool
		{
			mDescriptorPools.texture
				.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
				.Create(0, MAX_MESH_COUNT, nullptr); // each mesh will own a descriptor set for holding texture data

			mDescriptorSetLayouts.texture
				.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)	// sampler diffuse
				.Create(0, nullptr);
		}

		// cloth descriptors
		{
			mDescriptorPools.cloth
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
				.Create(0, static_cast<unsigned int>(mSwapchain->mImages.size()), nullptr);

			mDescriptorSetLayouts.cloth
				.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
				.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)
				.Create(0, nullptr);

			mDescriptorSets.cloth.Setup(mDescriptorSetLayouts.cloth.mLayout,
				mDescriptorPools.cloth.mPool,
				static_cast<unsigned int>(mSwapchain->mImages.size()));

			std::vector<VkWriteDescriptorSet> descriptorWrites = {};
			for (size_t i = 0; i < mDescriptorSets.cloth.mSets.size(); ++i)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = viewUboBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(viewUBO);

				VkWriteDescriptorSet writeBufferInfo = {};
				writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeBufferInfo.dstSet = mDescriptorSets.cloth.mSets[i];
				writeBufferInfo.dstBinding = 0;
				writeBufferInfo.dstArrayElement = 0;
				writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeBufferInfo.descriptorCount = 1;
				writeBufferInfo.pBufferInfo = &bufferInfo;

				descriptorWrites.push_back(writeBufferInfo);
			}
			vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

			descriptorWrites = {};
			for (size_t i = 0; i < mDescriptorSets.cloth.mSets.size(); ++i)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = lightUboBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(lightUBO);

				VkWriteDescriptorSet writeBufferInfo = {};
				writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeBufferInfo.dstSet = mDescriptorSets.cloth.mSets[i];
				writeBufferInfo.dstBinding = 1;
				writeBufferInfo.dstArrayElement = 0;
				writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeBufferInfo.descriptorCount = 1;
				writeBufferInfo.pBufferInfo = &bufferInfo;

				descriptorWrites.push_back(writeBufferInfo);
			}
			vkUpdateDescriptorSets(LOGICAL_DEVICE, static_cast<unsigned int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void ClothSimulation::SetupPipeline()
	{
		// mesh
		{
			mPipelineLayouts.mesh
				.AddDescriptorSetLayout(mDescriptorSetLayouts.mesh.mLayout)
				.AddDescriptorSetLayout(mDescriptorSetLayouts.texture.mLayout)
				.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelData))
				.Configure();

			mPipelines.mesh
				.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "cloth-simulation/mesh.vert.spv")
				.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "cloth-simulation/mesh.frag.spv")
				.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
				.AddVertexInputBindingDescription(vrassimp::MeshVertex::GetVertexInputBindingDescription())
				.AddVertexInputAttributeDescription(vrassimp::MeshVertex::GetVertexInputAttributeDescriptions())
				.ConfigureViewport(mSwapchain->mExtent)
				.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
				.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
				.ConfigureDefaultDepthTesting()
				.AddColorBlendAttachmentState(VK_FALSE, // disabled as clear color and object color were getting blended together
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

		// cloth
		{
			mPipelineLayouts.cloth
				.AddDescriptorSetLayout(mDescriptorSetLayouts.cloth.mLayout)
				.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelData))
				.Configure();

			mPipelines.cloth
				.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "cloth-simulation/cloth.vert.spv")
				.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "cloth-simulation/cloth.frag.spv")
				.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
				.AddVertexInputBindingDescription(ClothVertex::GetVertexInputBindingDescription())
				.AddVertexInputAttributeDescription(ClothVertex::GetVertexInputAttributeDescriptions())
				.ConfigureViewport(mSwapchain->mExtent)
				.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
				//.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
				.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
				.ConfigureDepthTesting(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL)
				//.ConfigureDefaultDepthTesting()
				.AddColorBlendAttachmentState(VK_FALSE, // disabled as clear color and object color were getting blended together
					VK_BLEND_FACTOR_SRC_ALPHA,
					VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					VK_BLEND_OP_ADD,
					VK_BLEND_FACTOR_SRC_ALPHA,
					VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					VK_BLEND_OP_SUBTRACT,
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
				.ConfigureColorBlendState(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
				.Configure(mPipelineLayouts.cloth.GetVulkanPipelineLayout(), mRenderpass.mRenderPass, 0, 0);
		}
	}

	void ClothSimulation::SetupUbo()
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

		bufferSize = sizeof(lightUBO);
		lightUboBuffers.resize(mSwapchain->mImages.size());
		lightUboMemory.resize(mSwapchain->mImages.size());
		for (unsigned int i = 0; i < mSwapchain->mImages.size(); ++i)
		{
			MemoryUtility::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ALLOCATION_CALLBACK, &lightUboBuffers[i], &lightUboMemory[i]);
		}
	}

	void ClothSimulation::SetupTextureSamplers()
	{
		mSamplers.diffuse.CreateDefault();
	}

	void ClothSimulation::LoadAssets()
	{
		//vrassimp::Model* floor = new vrassimp::Model();
		//{
		//	floor->LoadFromFile("dungeon_ground\\scene.gltf", "floor");
		//	floor->mTransform.position = glm::vec3(5.0f, -0.15f, -20.0f);
		//	floor->mTransform.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
		//	floor->mTransform.scale = glm::vec3(5.0f, 3.0f, 1.0f);
		//	floor->mAnimation = new vrassimp::Animation();
		//	mModels.push_back(floor);
		//}

		/*cloth = new vrassimp::Model();
		{
			cloth->LoadFromFile("plane\\plane.gltf", "plane");
			cloth->mTransform.position = glm::vec3(10.0f, 2.15f, -20.0f);
			cloth->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			cloth->mTransform.scale = glm::vec3(5.0f, 5.0f, 1.0f);
			cloth->mAnimation = new vrassimp::Animation();
			cloth->modelSettings.texturesAvailable = false;
			mModels.push_back(cloth);
		}*/

		ball = new vrassimp::Model();
		{
			ball->LoadFromFile("football\\scene.gltf", "football");
			ball->mTransform.position = glm::vec3(0.0f, 5.0f, -5.0f);
			ball->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			ball->mTransform.scale = glm::vec3(ballRadius);
			ball->mAnimation = new vrassimp::Animation();
			mModels.push_back(ball);
		}

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

	void ClothSimulation::UpdateViewBuffer(unsigned int imageIndex)
	{
		if (activeCamera == CameraType::LOOK_AT)
		{
			viewUBO.projection = eCamera.matrices.projection;
			viewUBO.view = eCamera.matrices.view;
		}
		else
		{
			viewUBO.projection = mCamera.project;
			viewUBO.view = mCamera.view;
		}

		void* data;
		vkMapMemory(LOGICAL_DEVICE, viewUboMemory[imageIndex], 0, sizeof(viewUBO), 0, &data);
		memcpy(data, &viewUBO, sizeof(viewUBO));
		vkUnmapMemory(LOGICAL_DEVICE, viewUboMemory[imageIndex]);
	}

	void ClothSimulation::UpdateModelData(vrassimp::Model* model, const double& frametime)
	{
		glm::mat4  modelMatrix(1.0);
		modelMatrix = glm::translate(modelMatrix, model->mTransform.position);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mTransform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mTransform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // y is rotated around z-axis
		modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mTransform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::scale(modelMatrix, model->mTransform.scale);
		modelData.model = modelMatrix;
	}

	void ClothSimulation::UpdateLightBuffer(unsigned int imageIndex)
	{
		void* data;
		vkMapMemory(LOGICAL_DEVICE, lightUboMemory[imageIndex], 0, sizeof(lightUBO), 0, &data);
		memcpy(data, &lightUBO, sizeof(lightUBO));
		vkUnmapMemory(LOGICAL_DEVICE, lightUboMemory[imageIndex]);
	}

	void ClothSimulation::RecordCommands(unsigned int imageIndex, const double& frametime)
	{
		UpdateViewBuffer(imageIndex);
		UpdateLightBuffer(imageIndex);
		UpdateCloth(imageIndex);

		// Information about how to begin each command buffer
		VkCommandBufferBeginInfo bufferBeginInfo = {};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Information about how to begin a render pass (only needed if we are doing a graphical application)
		std::array<VkClearValue, 2> clearValues = {};
		//clearValues[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
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

		//mCloth.UpdateCloth(ball->mTransform.position, ball->mTransform.scale.x);

		//draw models
		{
			for (auto& model : mModels)
			{
				UpdateModelData(model, frametime);

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

		DrawCloth(imageIndex);

		DrawUI(mGraphicsCommandBuffers[imageIndex]);

		// end render pass
		vkCmdEndRenderPass(mGraphicsCommandBuffers[imageIndex]);

		if (vkEndCommandBuffer(mGraphicsCommandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop recording a command buffer!!");
		}
	}

	void ClothSimulation::UpdateCloth(unsigned int imageIndex)
	{
		mCloth.addForce(glm::vec3(0, -0.02, 0.002) * Particle::TIME_STEP_2); // add gravity each frame, pointing down
		mCloth.windForce(glm::vec3(0.0, 0, -10.0) * Particle::TIME_STEP_2);
		mCloth.timeStep();
		mCloth.ballCollision(ball->mTransform.position, ball->mTransform.scale.x);
		std::vector<Particle>::iterator particle;
		for (particle = mCloth.particles.begin(); particle != mCloth.particles.end(); particle++)
		{
			(*particle).resetNormal();
		}
		std::vector<ClothVertex> vertexInputData;
		//std::vector<float> vertexInputData;
		//create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
		for (int x = 0; x < mCloth.num_particles_width - 1; x++)
		{
			for (int y = 0; y < mCloth.num_particles_height - 1; y++)
			{
				glm::vec3 normal = mCloth.calcTriangleNormal(mCloth.getParticle(x + 1, y), mCloth.getParticle(x, y), mCloth.getParticle(x, y + 1));
				mCloth.getParticle(x + 1, y)->addToNormal(normal);
				mCloth.getParticle(x, y)->addToNormal(normal);
				mCloth.getParticle(x, y + 1)->addToNormal(normal);
				Particle* part = mCloth.getParticle(x + 1, y);

				//vertexInputData.push_back(part->getPos().x);
				//vertexInputData.push_back(part->getPos().y);
				//vertexInputData.push_back(part->getPos().z);

				//vertexInputData.push_back(part->getNormal().x);
				//vertexInputData.push_back(part->getNormal().y);
				//vertexInputData.push_back(part->getNormal().z);

				//vertexInputData.push_back(part->getUV().x);
				//vertexInputData.push_back(part->getUV().y);

				vertexInputData.emplace_back(part->getPos(), part->getNormal(), part->getUV());
				////////////////////////////////////////
				part = mCloth.getParticle(x, y);

				//vertexInputData.push_back(part->getPos().x);
				//vertexInputData.push_back(part->getPos().y);
				//vertexInputData.push_back(part->getPos().z);

				//vertexInputData.push_back(part->getNormal().x);
				//vertexInputData.push_back(part->getNormal().y);
				//vertexInputData.push_back(part->getNormal().z);

				//vertexInputData.push_back(part->getUV().x);
				//vertexInputData.push_back(part->getUV().y);

				vertexInputData.emplace_back(part->getPos(), part->getNormal(), part->getUV());
				/////////////////////////////////////
				part = mCloth.getParticle(x, y + 1);

				//vertexInputData.push_back(part->getPos().x);
				//vertexInputData.push_back(part->getPos().y);
				//vertexInputData.push_back(part->getPos().z);

				//vertexInputData.push_back(part->getNormal().x);
				//vertexInputData.push_back(part->getNormal().y);
				//vertexInputData.push_back(part->getNormal().z);

				//vertexInputData.push_back(part->getUV().x);
				//vertexInputData.push_back(part->getUV().y);

				vertexInputData.emplace_back(part->getPos(), part->getNormal(), part->getUV());
				//////////////////////////////////////////////////////////////////////

				normal = mCloth.calcTriangleNormal(mCloth.getParticle(x + 1, y + 1), mCloth.getParticle(x + 1, y), mCloth.getParticle(x, y + 1));
				mCloth.getParticle(x + 1, y + 1)->addToNormal(normal);
				mCloth.getParticle(x + 1, y)->addToNormal(normal);
				mCloth.getParticle(x, y + 1)->addToNormal(normal);

				part = mCloth.getParticle(x + 1, y + 1);

				//vertexInputData.push_back(part->getPos().x);
				//vertexInputData.push_back(part->getPos().y);
				//vertexInputData.push_back(part->getPos().z);

				//vertexInputData.push_back(part->getNormal().x);
				//vertexInputData.push_back(part->getNormal().y);
				//vertexInputData.push_back(part->getNormal().z);

				//vertexInputData.push_back(part->getUV().x);
				//vertexInputData.push_back(part->getUV().y);

				vertexInputData.emplace_back(part->getPos(), part->getNormal(), part->getUV());
				////////////////////////////////////////
				part = mCloth.getParticle(x + 1, y);

				//vertexInputData.push_back(part->getPos().x);
				//vertexInputData.push_back(part->getPos().y);
				//vertexInputData.push_back(part->getPos().z);

				//vertexInputData.push_back(part->getNormal().x);
				//vertexInputData.push_back(part->getNormal().y);
				//vertexInputData.push_back(part->getNormal().z);

				//vertexInputData.push_back(part->getUV().x);
				//vertexInputData.push_back(part->getUV().y);

				vertexInputData.emplace_back(part->getPos(), part->getNormal(), part->getUV());
				/////////////////////////////////////
				part = mCloth.getParticle(x, y + 1);

				//vertexInputData.push_back(part->getPos().x);
				//vertexInputData.push_back(part->getPos().y);
				//vertexInputData.push_back(part->getPos().z);

				//vertexInputData.push_back(part->getNormal().x);
				//vertexInputData.push_back(part->getNormal().y);
				//vertexInputData.push_back(part->getNormal().z);

				//vertexInputData.push_back(part->getUV().x);
				//vertexInputData.push_back(part->getUV().y);

				vertexInputData.emplace_back(part->getPos(), part->getNormal(), part->getUV());
			}
		}

		//memcpy(mClothBuffer.mapped, vertexInputData.data(), vertexInputData.size() * sizeof(float));
		memcpy(mClothBuffer.mapped, vertexInputData.data(), vertexInputData.size() * sizeof(ClothVertex));
		mClothBuffer.CopyData(vertexInputData.data(), vertexInputData.size() * sizeof(ClothVertex));
	}

	void ClothSimulation::DrawCloth(unsigned int imageIndex)
	{
		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model, clothModel.position);
		model = glm::rotate(model, glm::radians(clothModel.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(clothModel.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // y is rotated around z-axis
		model = glm::rotate(model, glm::radians(clothModel.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, clothModel.scale);
		modelData.model = model;

		vkCmdBindPipeline(mGraphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.cloth.GetVulkanPipeline());

		vkCmdPushConstants(mGraphicsCommandBuffers[imageIndex],
			mPipelineLayouts.cloth.GetVulkanPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT, 0,
			sizeof(modelData), &modelData);	// we can also pass an array of data

		std::vector<VkDescriptorSet> descriptorSets = { mDescriptorSets.cloth.mSets[imageIndex] };
		vkCmdBindDescriptorSets(mGraphicsCommandBuffers[imageIndex],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayouts.cloth.GetVulkanPipelineLayout(),
			0,
			static_cast<unsigned int>(descriptorSets.size()),
			descriptorSets.data(), 0, nullptr);

		VkDeviceSize offsets[1] = { 0 };

		//// Bind mesh vertex buffer
		vkCmdBindVertexBuffers(mGraphicsCommandBuffers[imageIndex], 0, 1, &mClothBuffer.mBuffer, offsets);

		vkCmdDraw(mGraphicsCommandBuffers[imageIndex], 14406, 1, 0, 0);
	}

	Application* CreateApplication()
	{
		return new ClothSimulation("Cloth Simulation");
	}
}