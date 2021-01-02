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
	class VulkanBasics : public Application
	{
	public:

		VulkanBasics(std::string name);
		~VulkanBasics();

		virtual void InitializeScene() override;
		virtual void CleanupScene() override;
		virtual void Draw(const double& frametime) override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;

	private:

		void SetupPipeline();
		void RecordCommands(const unsigned int& currentImage);
		void UpdateUniformBuffer(const unsigned int& index);
		void SetupDescriptors();
		void SetupTextureSampler();

	private:

		Pipeline mPipeline;
		PipelineLayout mPipelineLayout;

		const std::vector<Vertex> VERTICES = {
			// Usually the front face setting in rasterizer governs the order of vertices in this array.
			// But applying projection and reversing y axis results in clockwise front facing even if
			// order of vertices here is clockwise.

			// set front face to VK_FRONT_FACE_CLOCKWISE in rasterizer configuration
			{{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},

			{{ 0.5f, -0.5f, 0.3f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f, 0.3f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f, 0.3f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f, 0.3f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}

			////////////////////////////////////////////////

				// set front face to VK_FRONT_FACE_COUNTER_CLOCKWISE in rasterizer configuration
				//{{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
				//{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
				//{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
				//{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
		};

		const std::vector<uint16_t> INDICES = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		Buffer<Vertex> mVertexBuffer;
		Buffer<uint16_t> mIndexBuffer;

		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 projection;
		} mvpBuffer;
		std::vector<VkBuffer> mMvpBuffers;// don't forget to destroy this
		std::vector<VkDeviceMemory> mMvpBuffersMemory; // don't forget to free this

		DescriptorSetLayout mDescriptorSetLayout;
		DescriptorPool mDescriptorPool;
		std::vector<VkDescriptorSet> mDescriptorSets;

		VulkanTexture mCross;
		TextureSampler mTextureSampler;

		int mCurrentFrame = 0;

		Clock<std::chrono::seconds::period> mClock;
	};
}