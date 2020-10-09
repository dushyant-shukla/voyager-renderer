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
#include "graphics/vulkan/Texture.h"

#include "clock/Clock.h"

namespace vr
{
	class AnimationKeyframes : public Application
	{
	public:

		AnimationKeyframes(std::string name);
		~AnimationKeyframes();

		virtual void InitializeScene() override;
		virtual void SetupPipeline() override;
		virtual void CleanupScene() override;
		virtual void Draw() override;
		void RecordCommands(const unsigned int& currentImage);

	private:

		void UpdateUniformBuffer(const unsigned int& index);
		void SetupDescriptors();

	private:

		Pipeline mPipeline;
		PipelineLayout mPipelineLayout;

		const std::vector<Vertex> VERTICES = {
			// Usually the front face setting in rasterizer governs the order of vertices in this array.
			// But applying projection and reversing y axis results in clockwise front facing even if
			// order of vertices here is clockwise.

			// set front face to VK_FRONT_FACE_CLOCKWISE in rasterizer configuration
			{{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}

			// set front face to VK_FRONT_FACE_COUNTER_CLOCKWISE in rasterizer configuration
			//{{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			//{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			//{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
			//{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
		};

		const std::vector<uint16_t> INDICES = {
			0, 1, 2, 2, 3, 0
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

		Texture cross;

		int mCurrentFrame = 0;

		Clock<std::chrono::seconds::period> mClock;
	};
}