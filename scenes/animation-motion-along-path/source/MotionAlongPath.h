#pragma once

#include "main.h"
#include "Application.h"
#include "Splines.h"
#include "camera/EditingModeCamera.h"

#define MAX_BONES 100

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
		void SetupDescriptorSetForCurve();
		void SetupPipelineForCurve();
		void SetupUboForCurve();

		// update methods
		void UpdateUniformBufferForCurve(unsigned int imageIndex);

		// draw commands
		void RecordCommands(unsigned int imageIndex);

	private:

		struct
		{
			PipelineLayout model;
			PipelineLayout curve;
		} mPipelineLayouts;

		struct
		{
			Pipeline model;
			Pipeline curve;
		} mPipelines;

		struct
		{
			DescriptorSets curve;
		} mDescriptorSets;

		struct
		{
			DescriptorSetLayout viewUBO;
		} mDescriptorSetLayouts;

		struct
		{
			DescriptorPool viewUBO;
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
			glm::mat4 model;
		} curvePushConstant;

		Splines* curve;

		int mCurrentFrame = 0;
	};
}
