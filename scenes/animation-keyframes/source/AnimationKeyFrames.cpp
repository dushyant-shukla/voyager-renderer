#include "AnimationKeyFrames.h"

namespace vr
{
	AnimationKeyframes::AnimationKeyframes(std::string name) : Application(name)
	{
	}

	void AnimationKeyframes::InitializeScene()
	{
		int a = 10;
	}

	Application* CreateApplication()
	{
		return new AnimationKeyframes("Animation (Key Frames)");
	}
}