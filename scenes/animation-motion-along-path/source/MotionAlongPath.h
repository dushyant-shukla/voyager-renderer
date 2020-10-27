#pragma once

#include "main.h"
#include "Application.h"
#include "Splines.h"
#include "graphics/vulkan/Model.h"

//#define MAX_BONES 100
#define MAX_MESH_COUNT 100

namespace vr
{
	class MotionAlongPath : public Application
	{
	public:

		MotionAlongPath(std::string name);
		~MotionAlongPath();

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
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 path;
			alignas(16) int enableAnimation;
		} modelData;

		struct
		{
			int isControlPoints;
		} primitiveModelData;

		struct
		{
			TextureSampler diffuse;
			TextureSampler specular;
			TextureSampler emission;
		} mSamplers;

		std::vector<vrassimp::Model*> mModels;

		Splines* curve;
		float t1, t2, t3, pathTime;
		float velocity = 6.0f;
		bool renderCurve = true;
		bool renderControlPoints = true;

		int mCurrentFrame = 0;
	};
}
