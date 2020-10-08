#pragma once

#include "main.h"

#include "Application.h"
#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/RenderPass.h"
#include "graphics/vulkan/Buffer.h"
#include "graphics/vulkan/Vertex.h"

namespace vr
{
	class AnimationKeyframes : public Application
	{
	public:

		AnimationKeyframes(std::string name);

		virtual void InitializeScene() override;
		virtual void SetupPipeline() override;
		virtual void CleanupScene() override;
		virtual void Draw() override;
		void RecordCommands(const unsigned int& currentImage);

	private:

		Pipeline mPipeline;
		PipelineLayout mPipelineLayout;

		const std::vector<Vertex> VERTICES = {
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

		int mCurrentFrame = 0;
	};
}