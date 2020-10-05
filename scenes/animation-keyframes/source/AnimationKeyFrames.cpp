#include "AnimationKeyFrames.h"
#include "graphics/vulkan/ShaderModule.h"
#include "graphics/vulkan/Device.h"

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name)/*,
		mPipeline(mDevice->GetLogicalDevice().device, nullptr)*/
	{}

	void AnimationKeyframes::InitializeScene()
	{
	}

	void AnimationKeyframes::SetupPipeline()
	{
		//mPipeline.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "assets/shaders/vert.spv");
		//mPipeline.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "assets/shaders/frag.spv");
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation (Key Frames)");
	}
}