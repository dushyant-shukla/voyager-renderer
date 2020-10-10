#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class CommandPool
	{
	public:

		CommandPool();
		~CommandPool();

		void Create(const int& queueFamilyIndex, VkCommandPoolCreateFlags flags);

		const VkCommandPool& GetVulkanCommandPool();

	private:

		VkCommandPool mPool = VK_NULL_HANDLE;
	};
}