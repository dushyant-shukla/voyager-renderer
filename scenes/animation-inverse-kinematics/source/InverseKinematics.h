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
#include "graphics/model/Model.h"

#define MAX_BONES 100

namespace vr
{
	class InverseKinematics : public Application
	{
	public:

		InverseKinematics(std::string name);
		~InverseKinematics();

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

		// input processing
		void ProcessInput(vrassimp::Model* model);

		// draw commands
		void RecordCommands(unsigned int imageIndex, const double& frametime);

	private:

		struct
		{
			PipelineLayout mesh;
			PipelineLayout curve;
		} mPipelineLayouts;

		struct
		{
			Pipeline mesh;
			Pipeline curve;
		} mPipelines;

		struct
		{
			DescriptorSets mesh;
			DescriptorSets curve;
		} mDescriptorSets;

		struct
		{
			DescriptorSetLayout mesh;
			DescriptorSetLayout curve;
			DescriptorSetLayout texture;
		} mDescriptorSetLayouts;

		struct
		{
			DescriptorPool mesh;
			DescriptorPool curve;
			DescriptorPool texture;
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
			alignas(16) glm::vec3 color;
			alignas(4) int enableAnimation;
			alignas(4) int texturesAvailable;
		} modelData;

		struct
		{
			TextureSampler diffuse;
			TextureSampler specular;
			TextureSampler emission;
		} mSamplers;

		vrassimp::Model* target;
		std::vector<vrassimp::Model*> mModels;

		float t1, t2, t3, pathTime;
		float velocity = 6.0f;
		bool renderCurve = true;
		bool renderControlPoints = true;

		int mCurrentFrame = 0;

		const int MAX_MESH_COUNT = 100;
	};
}