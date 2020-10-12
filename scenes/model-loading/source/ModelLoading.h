#pragma once

#include "main.h"
#include "Application.h"

namespace vr
{
	class ModelLoading : public Application
	{
	public:

		ModelLoading(std::string name);
		~ModelLoading();

		virtual void InitializeScene() override;
		virtual void CleanupScene() override;
		virtual void Draw() override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;
	};
}
