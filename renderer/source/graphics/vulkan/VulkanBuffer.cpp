#include "VulkanBuffer.h"

#include <cstring>

#define assert(expression) ((void)0)

namespace vr
{
	VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(LOGICAL_DEVICE, memory, 0, size, 0, &mapped);
	}

	void VulkanBuffer::Unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(LOGICAL_DEVICE, memory);
			mapped = nullptr;
		}
	}

	VkResult VulkanBuffer::Bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(LOGICAL_DEVICE, buffer, memory, offset);
	}

	void VulkanBuffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}

	void VulkanBuffer::CopyToMemory(void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	VkResult VulkanBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(LOGICAL_DEVICE, 1, &mappedRange);
	}

	VkResult VulkanBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(LOGICAL_DEVICE, 1, &mappedRange);
	}

	void VulkanBuffer::Destroy()
	{
		if (buffer)
		{
			vkDestroyBuffer(LOGICAL_DEVICE, buffer, nullptr);
		}
		if (memory)
		{
			vkFreeMemory(LOGICAL_DEVICE, memory, nullptr);
		}
	}
}