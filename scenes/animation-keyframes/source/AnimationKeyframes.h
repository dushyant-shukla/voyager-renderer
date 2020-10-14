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
		virtual void Draw() override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;

	private:

		void SetupPipeline();
		void RecordCommands(const unsigned int& currentImage);
		void SetupDescriptorSets();
		void SetupUniformBufferObjects();
		void SetupTextureSampler();
		void LoadAssets();
		void UpdateUniformBuffers(const unsigned int& imageIndex);

	private:

		struct
		{
			/*
				Uniform buffer data would be same for all geometries drawn in one frame
			*/
			DescriptorPool uniformBufferPool;
			DescriptorSetLayout uniformBufferLayout;
			std::vector<VkDescriptorSet> uniformBufferSets;

			/*
				Textures will change on per mesh basis.
				A descriptor set will be owned by each mesh with common pool and layout.
			*/
			DescriptorPool texturePool;
			DescriptorSetLayout textureLayout;
		} mDescriptors;

		struct
		{
			Pipeline pipeline;
		} mPipelines;

		struct
		{
			PipelineLayout pipelineLayout;
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
			alignas(16) glm::mat4 projectionViewMatrix;
			alignas(16) glm::vec3 viewPosition;
		} mViewUBO;
		std::vector<VkBuffer> mViewUboBuffers;// don't forget to destroy this
		std::vector<VkDeviceMemory> mViewUboBuffersMemory; // don't forget to free this

		/*
			Push constant descriptor
		*/
		struct
		{
			glm::mat4 model;
		} mPerModelData;

		std::vector<vrassimp::Model*> mModels;

		struct ImageInfo
		{
			int binding;
			VkDescriptorImageInfo info;
		};
		std::vector<ImageInfo> imageInfos;

		int mCurrentFrame = 0;
	};
}
