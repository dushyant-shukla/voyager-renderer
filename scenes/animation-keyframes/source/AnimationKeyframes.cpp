#include "AnimationKeyframes.h"

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name)
	{
	}

	AnimationKeyframes::~AnimationKeyframes()
	{
	}

	void AnimationKeyframes::InitializeScene()
	{
		vrassimp::Model::ModelCreateInfo modelCreateInfo(1.0, 1.0f, 0.0f);

		//mModels.resize(1);
		//mModels[0].LoadFromFile("ironman.fbx", &modelCreateInfo);

		vrassimp::Model model;
		model.LoadFromFile("wolf\\scene.gltf", &modelCreateInfo);

		mModels.push_back(std::move(model));
	}

	void AnimationKeyframes::CleanupScene()
	{
	}

	void AnimationKeyframes::Draw()
	{
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
			//.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "../../assets/shaders/vert.spv")
			//.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "../../assets/shaders/frag.spv")
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

	void AnimationKeyframes::SetupDescriptorSets()
	{
		// set up descriptor layout
		// layout binding descriptions: 1. uniform buffer for projection * view matrix and view position
		//								2. combined image sampler diffuse
		//								3. combined image sampler specular
		//								4. combined image sampler emission

		// setup pool size

		// allocate descriptor set

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

		std::vector<VkDescriptorSetLayout> layouts(mSwapchain->GetSwapchainImages().size(), mDescriptors.layout.GetVkDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptors.pool.GetVulkanDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->GetSwapchainImages().size());
		allocInfo.pSetLayouts = layouts.data();

		mDescriptors.sets.resize(mSwapchain->GetSwapchainImages().size());
		CHECK_RESULT(vkAllocateDescriptorSets(mDevice->GetLogicalDevice().device, &allocInfo, mDescriptors.sets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SETS");

		// TODO: configure descriptors (descriptor writes)

		// write buffer info

		// write image info for textures

		for (size_t modelIndex = 0; modelIndex < mModels.size(); ++modelIndex)
		{
			for (size_t meshIndex = 0; meshIndex < mModels[modelIndex].meshes.size(); ++meshIndex)
			{
				vrassimp::Mesh mesh = mModels[modelIndex].meshes[meshIndex];
				for (size_t textureIndex = 0; textureIndex < mesh.textures.size(); ++textureIndex)
				{
					Texture texture = mesh.textures[textureIndex].texture;
					//VkDescriptorImageInfo imageInfo = {};
					//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					//imageInfo.imageView = texture.GetVulkanImageView();
					//switch (mesh.textures[textureIndex].type)
					//{
					//case vrassimp::Texture::Type::DIFFUSE:
					//	imageInfo
					//}
					// propbably better to create info during conversion to vr::Texture from assimp texture
				}
			}
		}
	}

	void AnimationKeyframes::SetupUniformBufferObjects()
	{
	}

	void AnimationKeyframes::SetupTextureSampler()
	{
		mSamplers.diffuse.CreateDefault();
		mSamplers.specular.CreateDefault();
		mSamplers.emission.CreateDefault();
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation Key-frames");
	}
}