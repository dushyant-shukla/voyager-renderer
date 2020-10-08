#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class CommandPool
	{
	public:

		CommandPool();
		~CommandPool();

		void Create(const VkDevice& logicalDevice, VkAllocationCallbacks* allocationCallbacks, const int& queueFamilyIndex, VkCommandPoolCreateFlags flags);

		const VkCommandPool& GetVulkanCommandPool();

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		VkCommandPool mPool;
	};
}