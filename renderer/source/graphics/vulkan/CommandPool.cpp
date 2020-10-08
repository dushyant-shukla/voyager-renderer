#include "CommandPool.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	CommandPool::CommandPool()
	{
	}

	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(mLogicalDevice, mPool, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: COMMAND POOL");
	}

	void CommandPool::Create(const VkDevice& logicalDevice, VkAllocationCallbacks* allocationCallbacks, const int& queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		mLogicalDevice = logicalDevice;
		mAllocationCallbacks = allocationCallbacks;

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = flags;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		CHECK_RESULT(vkCreateCommandPool(mLogicalDevice, &poolInfo, mAllocationCallbacks, &mPool), "RESOURCE CREATION FAILED: COMMAND POOL");
		RENDERER_DEBUG("RESOURCE CREATED: COMMAND POOL");
	}

	const VkCommandPool& CommandPool::GetVulkanCommandPool()
	{
		return mPool;
	}
}