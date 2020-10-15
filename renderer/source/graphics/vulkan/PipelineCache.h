#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class PipelineCache
	{
	public:

		PipelineCache();
		~PipelineCache();

		void Setup();

		VkPipelineCache mCache = VK_NULL_HANDLE;
	};
}
