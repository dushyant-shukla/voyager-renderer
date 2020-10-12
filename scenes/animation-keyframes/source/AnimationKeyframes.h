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

	private:

		struct
		{
			DescriptorPool pool;
			DescriptorSetLayout layout;
			std::vector<VkDescriptorSet> sets;
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
			glm::mat4 projectionViewMatrix;
			glm::vec3 viewPosition;
		} mViewUBO;

		/*
			Push constant descriptor
		*/
		struct
		{
		} mPerModelData;

		std::vector<vrassimp::Model> mModels;
	};
}
