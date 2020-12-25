#include "InverseKinematics.h"

#include "graphics/vulkan/utility/RendererCoreUtility.h"
#include "input/InputManager.h"

#include <imgui.h>

vr::InverseKinematics::InverseKinematics(std::string name) : Application(name)
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
	lightUBO.constant = 1.0f;
	lightUBO.linear = 0.22f;
	lightUBO.quadratic = 0.20f;
}

vr::InverseKinematics::~InverseKinematics()
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
			vkDestroyBuffer(LOGICAL_DEVICE, lightUboBuffers[i], nullptr);
			vkFreeMemory(LOGICAL_DEVICE, lightUboMemory[i], nullptr);
		}
		RENDERER_DEBUG("RESOURCE DESTROYED: UNIFORM BUFFER");
		RENDERER_DEBUG("RESOURCE FREED: UNIFORM BUFFER MEMORY");
	}
}

void vr::InverseKinematics::InitializeScene()
{
	SetupTextureSamplers();
	SetupDescriptorSet();
	SetupPipeline();
	LoadAssets();
	isReady = true;
}

void vr::InverseKinematics::CleanupScene()
{
}

void vr::InverseKinematics::Draw(const double& frametime)
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

VkPhysicalDeviceFeatures vr::InverseKinematics::CheckRequiredFeatures()
{
	bool requiredFeaturesAvailable = true;
	VkPhysicalDeviceFeatures requiredDeviceFeatures = {};
	if (!(mDevice->GetPhysicalDevice().features.samplerAnisotropy))
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

void vr::InverseKinematics::SetupDescriptorSet()
{
	SetupUbo();

	// mesh descriptor sets
	{
		mDescriptorPools.mesh
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.Create(0, static_cast<unsigned int>(mSwapchain->mImages.size()), nullptr);

		mDescriptorSetLayouts.mesh
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
			.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr)
			.AddLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)
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
			writeBufferInfo.dstBinding = 2;
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

void vr::InverseKinematics::SetupPipeline()
{
	// pipeline for mesh
	{
		mPipelineLayouts.mesh
			.AddDescriptorSetLayout(mDescriptorSetLayouts.mesh.mLayout)
			.AddDescriptorSetLayout(mDescriptorSetLayouts.texture.mLayout)
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelData))
			.Configure();

		mPipelines.mesh
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "inverse-kinematics/vertex-skinning.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "inverse-kinematics/vertex-skinning.frag.spv")
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
}

void vr::InverseKinematics::SetupUbo()
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

void vr::InverseKinematics::SetupTextureSamplers()
{
	mSamplers.diffuse.CreateDefault();
	mSamplers.specular.CreateDefault();
	mSamplers.emission.CreateDefault();
}

void vr::InverseKinematics::LoadAssets()
{
	// floor
	{
		vrassimp::Model* floor = new vrassimp::Model();
		floor->LoadFromFile("floor\\scene.gltf", "floor");
		floor->mTransform.position = glm::vec3(0.0f, -0.15f, 0.0f);
		floor->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		floor->mTransform.scale = glm::vec3(6.0f, 3.0f, 6.0f);
		mModels.push_back(floor);
	}

	// nathan
	{
		//vrassimp::Model* nathan = new vrassimp::Model();
		//nathan->LoadFromFile("nathan\\scene.gltf", "nathan");
		//nathan->mTransform.position = glm::vec3(0.0f, 0.2f, 0.0f);
		//nathan->mTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
		//nathan->mTransform.scale = glm::vec3(0.05, 0.05f, 0.05f);
		//nathan->mAnimationTransform.position = glm::vec3(0.0f, 0.2f, 0.0f);
		//nathan->mAnimationTransform.rotation = glm::vec3(90.0f, 0.0f, 180.0f);
		//nathan->mAnimationTransform.scale = glm::vec3(0.05, 0.05f, 0.05f);
		//nathan->mAnimation->settings.speed = 25.0f;
		//mModels.push_back(nathan);
	}

	{
		vrassimp::Model* spidey = new vrassimp::Model();
		spidey->LoadFromFile("spiderman\\spiderman.fbx", "spidey");
		spidey->mTransform.position = glm::vec3(0.0f, 0.1f, 0.0f);
		spidey->mTransform.rotation = glm::vec3(-90.0f, 180.0f, 0.0f);
		spidey->mTransform.scale = glm::vec3(350.0, 350.0f, 350.0f);
		spidey->mAnimationTransform.position = glm::vec3(0.0f, 0.1f, 3.0f);
		spidey->mAnimationTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		spidey->mAnimationTransform.scale = glm::vec3(3.0, 3.0f, 3.0f);
		spidey->mAnimation->settings.speed = 0.75f;

		spidey->modelSettings.texturesAvailable = 1;
		spidey->modelSettings.color = glm::vec3(1.0, 0.0, 0.0);
		//spidey->mAnimation->mIKBoneNames.push_back("mixamorig_RightHandIndex1");
		//spidey->mAnimation->mIKBoneNames.push_back("mixamorig_RightShoulder");
		//spidey->mAnimation->mIKBoneNames.push_back("mixamorig_RightArm");
		//spidey->mAnimation->mIKBoneNames.push_back("mixamorig_RightForeArm");
		//spidey->mAnimation->mIKBoneNames.push_back("mixamorig_RightHand");
		//spidey->mAnimation->mCCDSolver.mIkBoneMap.emplace(std::make_pair("mixamorig_RightShoulder", 0));
		//spidey->mAnimation->mCCDSolver.mIkBoneMap.emplace(std::make_pair("mixamorig_RightArm", 0));
		//spidey->mAnimation->mCCDSolver.mIkBoneMap.emplace(std::make_pair("mixamorig_RightForeArm", 0));
		//spidey->mAnimation->mCCDSolver.mIkBoneMap.emplace(std::make_pair("mixamorig_RightHand", 0));
		//spidey->mAnimation->mCCDSolver.mIkBoneMap.emplace(std::make_pair("mixamorig_RightHandIndex1", 0));

		spidey->mAnimation->mCCDSolver.AddIkBone("mixamorig_RightShoulder");
		spidey->mAnimation->mCCDSolver.AddIkBone("mixamorig_RightArm");
		spidey->mAnimation->mCCDSolver.AddIkBone("mixamorig_RightForeArm");
		spidey->mAnimation->mCCDSolver.AddIkBone("mixamorig_RightHand");
		spidey->mAnimation->mCCDSolver.AddIkBone("mixamorig_RightHandIndex1");
		spidey->mAnimation->InitializeIKData();

		mModels.push_back(spidey);
	}

	{
		target = new vrassimp::Model();
		target->LoadFromFile("sphere\\sm_sphere.fbx", "target");
		target->mTransform.position = glm::vec3(2.0f, 4.0f, 0.0f);
		target->mTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		target->mTransform.scale = glm::vec3(10.0, 10.0f, 10.0f);
		target->modelSettings.texturesAvailable = 0;
		target->modelSettings.color = glm::vec3(1.0, 0.0, 0.0);
		mModels.push_back(target);
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

void vr::InverseKinematics::UpdateViewBuffer(unsigned int imageIndex)
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

void vr::InverseKinematics::UpdateModelData(vrassimp::Model* model, const double& frametime)
{
	if (model->isAnimationAvailable)
	{
		// if model has animation, take current state from UI's animation setting
		//modelData.enableAnimation = model->mAnimation->settings.enableAnimation;
		//if (modelData.enableAnimation)
		//{
		//	glm::mat4  modelMatrix(1.0);
		//	modelMatrix = glm::translate(modelMatrix, model->mAnimationTransform.position);
		//	modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mAnimationTransform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//	modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mAnimationTransform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // y is rotated around z-axis
		//	modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mAnimationTransform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		//	modelMatrix = glm::scale(modelMatrix, model->mAnimationTransform.scale);
		//	modelData.model = modelMatrix;
		//	return;
		//}
		modelData.enableAnimation = model->mAnimation->settings.enableIk;
	}
	else
	{
		modelData.enableAnimation = 0;
	}
	glm::mat4  modelMatrix(1.0);
	modelMatrix = glm::translate(modelMatrix, model->mTransform.position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mTransform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mTransform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // y is rotated around z-axis
	modelMatrix = glm::rotate(modelMatrix, glm::radians(model->mTransform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, model->mTransform.scale);
	modelData.model = modelMatrix;

	modelData.texturesAvailable = model->modelSettings.texturesAvailable;
	modelData.color = model->modelSettings.color;
}

void vr::InverseKinematics::UpdateBoneTransforms(vrassimp::Model* model, unsigned int imageIndex, const double& frametime)
{
	vrassimp::Animation* animation = model->mAnimation;

	// keeps looping the current animation
	if (animation->timer > animation->availableTracks[animation->settings.currentTrackIndex].end)
	{
		animation->timer = animation->availableTracks[animation->settings.currentTrackIndex].start;
	}

	std::vector<aiMatrix4x4> transforms; // joint transforms in model-space
	std::vector<aiMatrix4x4> boneTransforms;
	//model->mAnimation->Animate(animation->timer, transforms, boneTransforms);
	model->mAnimation->Animate(0.0, transforms, boneTransforms);
	for (unsigned int i = 0; i < transforms.size(); ++i)
	{
		skinningUBO.bones[i] = glm::transpose(glm::make_mat4(&(transforms[i].a1)));
	}

	// solve CCD
	model->mAnimation->mCCDSolver.Solve(target->mTransform.position);
	for (unsigned int i = 0; i < model->mAnimation->mBoneCount; ++i)
	{
		glm::mat4 transform = model->mAnimation->mCCDSolver.GetGlobalTransformByBoneId(i);
		skinningUBO.bones[i] = transform;
	}

	void* data;
	vkMapMemory(LOGICAL_DEVICE, skinningUboMemory[imageIndex], 0, sizeof(skinningUBO), 0, &data);
	memcpy(data, &skinningUBO, sizeof(skinningUBO));
	vkUnmapMemory(LOGICAL_DEVICE, skinningUboMemory[imageIndex]);

	animation->timer += frametime * model->mAnimation->settings.speed;
}

void vr::InverseKinematics::UpdateLightBuffer(unsigned int imageIndex)
{
	void* data;
	vkMapMemory(LOGICAL_DEVICE, lightUboMemory[imageIndex], 0, sizeof(lightUBO), 0, &data);
	memcpy(data, &lightUBO, sizeof(lightUBO));
	vkUnmapMemory(LOGICAL_DEVICE, lightUboMemory[imageIndex]);
}

void vr::InverseKinematics::ProcessInput(vrassimp::Model* model)
{
	if (model->mScreenName._Equal("target"))
	{
		InputManager* inputManager = InputManager::GetInstance();
		if (inputManager->IsKeyPressed(KEY_UP))
		{
			model->mTransform.position.y += 0.01;
		}
		if (inputManager->IsKeyPressed(KEY_DOWN))
		{
			model->mTransform.position.y -= 0.01;
		}

		if (inputManager->IsKeyPressed(KEY_LEFT))
		{
			model->mTransform.position.x -= 0.01;
		}
		if (inputManager->IsKeyPressed(KEY_RIGHT))
		{
			model->mTransform.position.x += 0.01;
		}

		if (inputManager->IsKeyPressed(KEY_PAGE_UP))
		{
			model->mTransform.position.z += 0.01;
		}
		if (inputManager->IsKeyPressed(KEY_PAGE_DOWN))
		{
			model->mTransform.position.z -= 0.01;
		}
	}
}

void vr::InverseKinematics::RecordCommands(unsigned int imageIndex, const double& frametime)
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

	UpdateViewBuffer(imageIndex);

	// draw models
	{
		for (auto& model : mModels)
		{
			ProcessInput(model);
			UpdateModelData(model, frametime);
			if (model->isAnimationAvailable && (model->mAnimation->settings.enableIk == 1 || model->mAnimation->settings.enableAnimation == 1))
			{
				UpdateBoneTransforms(model, imageIndex, frametime);
			}

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

void vr::InverseKinematics::OnUpdateUIOverlay(UiOverlay* overlay)
{
	ImGui::SetNextWindowPos(ImVec2(10, 210));
	ImGui::Begin("Animation (Inverse Kinematics)");
	ImGui::Text("Settings:");
	if (ImGui::TreeNode("Models"))
	{
		for (auto& model : mModels)
		{
			bool showStaticTransforms = true;
			if (ImGui::TreeNode(model->mScreenName.c_str()))
			{
				if (model->isAnimationAvailable)
				{
					if (overlay->CheckBox("Enable animation", &(model->mAnimation->settings.enableAnimation)))
					{
					}
					if (overlay->CheckBox("Enable IK", &(model->mAnimation->settings.enableIk)))
					{
					}
					if (model->mAnimation->settings.enableAnimation &&
						overlay->InputFloat("speed", &(model->mAnimation->settings.speed), 0.5, 3))
					{
					}
					if (model->mAnimation->settings.enableAnimation)
					{
						ImGui::Separator();
						ImGui::Text("current speed: %.3f", model->mAnimation->settings.currentSpeed);
						ImGui::Separator();
					}
					/*if (model->mAnimation->settings.enableAnimation &&
						overlay->InputFloat("curve velocity", &velocity, 0.01, 3))
					{
					}*/
					if (model->mAnimation->settings.enableAnimation &&
						ImGui::Combo("track", &(model->mAnimation->settings.currentTrackIndex), model->mAnimation->settings.tracks.c_str()))
					{
					}
					if (model->mAnimation->settings.enableAnimation)
					{
						showStaticTransforms = false;

						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##scale_x", &(model->mAnimationTransform.scale.x), 0.01f, -9999.0, 9999.0, "S:x = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##scale_y", &(model->mAnimationTransform.scale.y), 0.01f, -9999.0, 9999.0, "S:y = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##scale_z", &(model->mAnimationTransform.scale.z), 0.01f, -9999.0, 9999.0, "S:z = %.2f");
						ImGui::PopItemWidth();

						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##rotate_x", &(model->mAnimationTransform.rotation.x), 0.05f, -9999.0, 9999.0, "R:x = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##rotate_y", &(model->mAnimationTransform.rotation.y), 0.05f, -9999.0, 9999.0, "R:y = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##rotate_z", &(model->mAnimationTransform.rotation.z), 0.05f, -9999.0, 9999.0, "R:z = %.2f");
						ImGui::PopItemWidth();

						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##translate_x", &(model->mAnimationTransform.position.x), 0.05f, -9999.0, 9999.0, "T:x = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##translate_y", &(model->mAnimationTransform.position.y), 0.05f, -9999.0, 9999.0, "T:y = %.2f");
						ImGui::PopItemWidth();
						ImGui::SameLine();
						ImGui::PushItemWidth(80);
						ImGui::DragFloat("##translate_z", &(model->mAnimationTransform.position.z), 0.05f, -9999.0, 9999.0, "T:z = %.2f");
						ImGui::PopItemWidth();
					}
				}

				// static transforms
				if (showStaticTransforms)
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
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
	ImGui::End();
}

vr::Application* vr::CreateApplication()
{
	return new InverseKinematics("Inverse Kinematics");
}