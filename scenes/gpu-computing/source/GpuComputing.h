#pragma once

#include "main.h"
#include "Application.h"

#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/RenderPass.h"
#include "graphics/vulkan/Buffer.h"
#include "graphics/vulkan/Vertex.h"
#include "graphics/vulkan/DescriptorSetLayout.h"
#include "graphics/vulkan/DescriptorPool.h"
#include "graphics/vulkan/VulkanTexture.h"
#include "graphics/vulkan/TextureSampler.h"

#include "clock/Clock.h"

namespace vr
{
	class GpuComputing : public Application
	{
	public:

		GpuComputing(std::string name);
		~GpuComputing();

		virtual void InitializeScene() override;
		virtual void CleanupScene() override;
		virtual void Draw(const double& frametime) override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;
		virtual void OnUpdateUIOverlay(UiOverlay* overlay) override;

	private:

		void SetupPipeline();
		void RecordCommands(const unsigned int& currentImage);
		void UpdateUniformBuffer(const unsigned int& index);
		void SetupDescriptors();
		void SetupTextureSampler();

		// compute shader related methods
		void Compute();
		void PrepareComputeCommandBuffer();
		void BeginComputeCommandBuffer();
		void RecordComputeCommands();
		void EndComputerCommandBuffer();

	private:

		Pipeline mPipeline;
		PipelineLayout mPipelineLayout;

		VkPipeline mComputePipeline;
		CommandBuffers mComputeCommandBuffer;

		int mCurrentFrame = 0;
		Clock<std::chrono::seconds::period> mClock;
	};
}