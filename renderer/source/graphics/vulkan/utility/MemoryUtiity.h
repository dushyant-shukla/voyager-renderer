#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class MemoryUtiity
	{
	public:

		static unsigned int FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int allowedTypes, VkMemoryPropertyFlags propertyFlags);
	};
}
