#pragma once

#include "main.h"

#include "Application.h"
#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/RenderPass.h"

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
		int mCurrentFrame = 0;
	};
}