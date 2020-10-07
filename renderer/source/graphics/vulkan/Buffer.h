#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	template <typename T>
	class Buffer
	{
	public:

	private:

	private:

		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
	};
}
