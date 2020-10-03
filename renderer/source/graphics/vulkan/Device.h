#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class Device
	{
	private:

		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicalDevice;
	};
}
