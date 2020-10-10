#include "CommandPool.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	CommandPool::CommandPool()
	{
	}

	CommandPool::~CommandPool()
	{
		if (mPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(LOGICAL_DEVICE, mPool, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: COMMAND POOL");
		}
	}

	void CommandPool::Create(const int& queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = flags;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		CHECK_RESULT(vkCreateCommandPool(LOGICAL_DEVICE, &poolInfo, ALLOCATION_CALLBACK, &mPool), "RESOURCE CREATION FAILED: COMMAND POOL");
		RENDERER_DEBUG("RESOURCE CREATED: COMMAND POOL");
	}

	const VkCommandPool& CommandPool::GetVulkanCommandPool()
	{
		return mPool;
	}
}