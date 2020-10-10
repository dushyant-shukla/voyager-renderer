#include "CommandBuffers.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	CommandBuffers::CommandBuffers()
	{
	}

	CommandBuffers::~CommandBuffers()
	{
		if (mCommandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(LOGICAL_DEVICE, mCommandPool, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: COMMAND POOL");
		}
	}

	void CommandBuffers::Create(const int& queueFamilyIndex, const int& count)
	{
		CreateCommandPool(queueFamilyIndex);
		AllocateCommandbuffers(count);
	}

	VkCommandBuffer& CommandBuffers::operator[](const unsigned int& index)
	{
		return mCommandbuffers[index];
	}

	size_t CommandBuffers::Size()
	{
		return mCommandbuffers.size();
	}

	void CommandBuffers::CreateCommandPool(const int& queueFamilyIndex)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	// this flag forces a reset on command buffer on every vkCommandBuffer() (check record command function)
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		CHECK_RESULT(vkCreateCommandPool(LOGICAL_DEVICE, &poolInfo, ALLOCATION_CALLBACK, &mCommandPool), "RESOURCE CREATION FAILED: COMMAND POOL");
		RENDERER_DEBUG("RESOURCE CREATED: COMMAND POOL");
	}

	void CommandBuffers::AllocateCommandbuffers(const int& count)
	{
		mCommandbuffers.resize(count);

		VkCommandBufferAllocateInfo cAllocateInfo = {};
		cAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cAllocateInfo.commandPool = mCommandPool;
		cAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Buffer you submit directly queue, Cannot be called by another buffer
																	// VK_COMMAND_BUFFER_LEVEL_SECONDARY: Buffer can't be called directly. Can be called from other buffers via "vkCmdExecuteCommands()" when recording commands in primary buffer
		cAllocateInfo.commandBufferCount = static_cast<unsigned int> (mCommandbuffers.size());

		// Allocate command buffers and place the handles in an array of buffers
		CHECK_RESULT(vkAllocateCommandBuffers(LOGICAL_DEVICE, &cAllocateInfo, mCommandbuffers.data()), "RESOURCE ALLOCATION FAILED: COMMAND BUFFERS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: COMMAND BUFFERS");
	}
}