#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "main.h"
#include "Application.h"
#include "graphics/vulkan/DescriptorPool.h"
#include "graphics/vulkan/DescriptorSetLayout.h"
#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/TextureSampler.h"
#include "graphics/vulkan/Model.h"

#define MAX_BONES 100

namespace vr
{
	class AnimationKeyframes : public Application
	{
	public:

		AnimationKeyframes(std::string name);
		~AnimationKeyframes();

		virtual void InitializeScene() override;
		virtual void CleanupScene() override;
		virtual void Draw(const double& frametime) override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;
		virtual void OnUpdateUIOverlay(UiOverlay* overlay) override;

	private:

		// setup methods
		void SetupDescriptorSet();
		void SetupPipeline();
		void SetupUbo();
		void SetupTextureSamplers();

		// loading assets
		void LoadAssets();

		// update methods
		void UpdateViewBuffer(unsigned int imageIndex);
		void UpdateModelData(vrassimp::Model* model, const double& frametime);
		void UpdateBoneTransforms(vrassimp::Model* model, unsigned int imageIndex, const double& frametime);
		void UpdateLightBuffer(unsigned int imageIndex);

		// draw commands
		void RecordCommands(unsigned int imageIndex, const double& frametime);

	private:

		struct
		{
			PipelineLayout mesh;
			PipelineLayout debug;
			PipelineLayout lines;
		} mPipelineLayouts;

		struct
		{
			Pipeline mesh;
			Pipeline debug;
			Pipeline lines;
		} mPipelines;

		struct
		{
			DescriptorSets mesh;
			DescriptorSets debug;
		} mDescriptorSets;

		struct
		{
			DescriptorSetLayout mesh;
			DescriptorSetLayout texture;
			DescriptorSetLayout debug;
		} mDescriptorSetLayouts;

		struct
		{
			DescriptorPool mesh;
			DescriptorPool texture;
			DescriptorPool debug;
		} mDescriptorPools;

		struct
		{
			alignas(16) glm::mat4 projection;
			alignas(16) glm::mat4 view;
		} viewUBO;
		std::vector<VkBuffer> viewUboBuffers;
		std::vector<VkDeviceMemory> viewUboMemory;

		struct
		{
			alignas(16) glm::mat4 bones[MAX_BONES];
		} skinningUBO;
		std::vector<VkBuffer> skinningUboBuffers;
		std::vector<VkDeviceMemory> skinningUboMemory;

		Buffer<glm::vec4> jointVertexBuffer;
		Buffer<glm::vec4> boneVertexBuffer;

		struct
		{
			alignas(16) glm::vec3 position;
			alignas(16) glm::vec3 color;
			//alignas(16) glm::vec3 direction;
			//alignas(4) float cutoff;
			alignas(4) float constant;
			alignas(4) float linear;
			alignas(4) float quadratic;
		} lightUBO;
		std::vector<VkBuffer> lightUboBuffers;
		std::vector<VkDeviceMemory> lightUboMemory;

		struct
		{
			alignas(16) glm::mat4 model;
			alignas(8) glm::vec2 uvOffset = glm::vec2(0.0f);
			alignas(4) int enableAnimation;
			alignas(4) int type;
		} modelData;

		struct BoneModelData
		{
			alignas(16) glm::mat4 jointModel;
			alignas(16) glm::mat4 lineModel;
			alignas(4) int renderJoints;
		} boneModelData;

		struct
		{
			TextureSampler diffuse;
			TextureSampler specular;
			TextureSampler emission;
		} mSamplers;

		vrassimp::Model* animatedModel;
		vrassimp::Model* floor;
		std::vector<vrassimp::Model*> mModels;

		int mCurrentFrame = 0;

		const int MAX_MESH_COUNT = 100;
	};
}