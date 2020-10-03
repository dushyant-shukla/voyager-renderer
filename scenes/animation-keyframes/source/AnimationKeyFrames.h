#pragma once

#include "main.h"

#include "Application.h"

namespace vr
{
	class AnimationKeyframes : public Application
	{
	public:

		AnimationKeyframes(std::string name);

		virtual void InitializeScene() override;
	};
}