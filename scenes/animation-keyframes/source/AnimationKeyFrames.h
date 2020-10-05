#pragma once

#include "main.h"

#include "Application.h"
#include "graphics/vulkan/Pipeline.h"

namespace vr
{
	class AnimationKeyframes : public Application
	{
	public:

		AnimationKeyframes(std::string name);

		virtual void InitializeScene() override;
		virtual void SetupPipeline() override;

	private:

		//Pipeline mPipeline;
	};
}