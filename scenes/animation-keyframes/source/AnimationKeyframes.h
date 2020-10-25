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
	class AnimationKeyframes
		: public Application
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

		void SetupPipeline();
		void RecordCommands(const unsigned int& currentImage);
		void SetupDescriptorSets();
		void SetupUniformBufferObjects();
		void SetupTextureSampler();
		void LoadAssets();
		void UpdateUniformBuffers(const unsigned int& imageIndex);
		void UpdateModelInfo(vrassimp::Model* model);
		glm::mat4 aiMatrix4x4ToGlm(aiMatrix4x4 ai_matr);
		void UpdateBoneTransforms();
		void Deserialize(const std::string archetype);

	private:

		struct
		{
			/*
				Uniform buffer data would be same for all geometries drawn in one frame.
				Used by mesh pipeline.
			*/
			DescriptorPool meshUboPool;
			DescriptorSetLayout meshUboLayout;
			std::vector<VkDescriptorSet> meshUboSets;

			/*
				Textures will change on per mesh basis.
				A descriptor set will be owned by each mesh with common pool and layout.
				Used by mesh pipeline.
			*/
			DescriptorPool texturePool;
			DescriptorSetLayout textureLayout;

			/*
				Used by primitive (bones and joints) draw pipelines
			*/
			DescriptorPool primitiveUboPool;
			DescriptorSetLayout primitiveUboLayout;
			std::vector<VkDescriptorSet> primitiveUboSets;
		} mDescriptors;

		struct
		{
			Pipeline mesh;
			Pipeline bones; // lines
			Pipeline joints; // points
		} mPipelines;

		struct
		{
			PipelineLayout mesh;
			PipelineLayout joints;
			PipelineLayout bones;
		} mPipelineLayouts;

		struct
		{
			TextureSampler diffuse;
			TextureSampler specular;
			TextureSampler emission;
		} mSamplers;

		/*
		* Uniform buffer descriptor
		*/
		struct
		{
			alignas(16) glm::mat4 projection;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 bones[MAX_BONES];
			alignas(16) glm::vec3 viewPosition;
		} meshUBO;
		std::vector<VkBuffer> meshUboBuffers;// don't forget to destroy this
		std::vector<VkDeviceMemory> meshUboMemory; // don't forget to free this

		struct
		{
			alignas(16) glm::mat4 projection;
			alignas(16) glm::mat4 view;
		} primitiveUBO;
		std::vector<VkBuffer> primitiveUboBuffers;// don't forget to destroy this
		std::vector<VkDeviceMemory> primitiveUboMemory; // don't forget to free this

		Buffer<glm::vec4> jointVertexBuffer;
		Buffer<glm::vec4> boneVertexBuffer;

		struct
		{
			glm::mat4 model;
		} mPrimtiveModelData;

		std::vector<aiMatrix4x4> boneTransforms;

		float timer = 0;

		/*
			Push constant descriptor
		*/
		struct
		{
			glm::mat4 model;
			int enableAnimation = 1;
		} mPerModelData;

		std::vector<vrassimp::Model*> mModels;

		struct
		{
			std::string name;
		} archetypeInfo;

		struct
		{
			float speed = 0.75;
			int id = 0;
			std::string animations = "";
			int boneLine = 0;
		} animationSettings;

		int mCurrentFrame = 0;
	};
}
