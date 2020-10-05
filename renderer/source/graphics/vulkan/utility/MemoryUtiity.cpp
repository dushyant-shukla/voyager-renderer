#include "MemoryUtiity.h"

namespace vr
{
	unsigned int MemoryUtiity::FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int allowedTypes, VkMemoryPropertyFlags propertyFlags)
	{
		// Get properties of the physical device memory
		VkPhysicalDeviceMemoryProperties memoryProperties = {};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		for (unsigned int i = 0; i < memoryProperties.memoryTypeCount; ++i)
		{
			// index of memory types must match corresponding bit in allowed types
			// desired property bit flags are part of memory type's property flags
			if ((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags))
			{
				return i;	// this memory type is valid so return its index
			}
		}
	}
}