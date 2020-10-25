#pragma once

#include "main.h"
#include "Application.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/DescriptorPool.h"
#include "graphics/vulkan/DescriptorSets.h"
#include "graphics/vulkan/DescriptorSetLayout.h"

namespace vr
{
	class ModelLoading : public Application
	{
	public:

		ModelLoading(std::string name);
		~ModelLoading();

		virtual void InitializeScene() override;
		virtual void CleanupScene() override;
		virtual void Draw(const double& frametime) override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;
	};
}
