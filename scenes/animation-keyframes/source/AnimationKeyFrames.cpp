#include "AnimationKeyFrames.h"
#include "graphics/vulkan/ShaderModule.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name),
		mPipeline(),
		mPipelineLayout()
	{}

	void AnimationKeyframes::InitializeScene()
	{
		SetupPipeline();
	}

	void AnimationKeyframes::SetupPipeline()
	{
		mPipelineLayout.Create(mDevice->GetLogicalDevice().device, nullptr)
			.Configure();

		mPipeline.Create(mDevice->GetLogicalDevice().device, nullptr)
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "../../assets/shaders/vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "../../assets/shaders/frag.spv");
	}

	void AnimationKeyframes::CleanupScene()
	{
		mPipelineLayout.Cleanup();
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation (Key Frames)");
	}
}