#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "utility/RendererCoreUtility.h"
#include "utility/MemoryUtility.h"
#include "RendererState.h"

namespace vr
{
	template <typename T>
	class Buffer
	{
	public:

		Buffer();
		~Buffer();

		void Create(const std::vector<T>& vertices, VkBufferUsageFlagBits flags);

		const VkBuffer& GetVulkanBuffer();

		VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void     Unmap();
		VkResult Bind(VkDeviceSize offset = 0);
		void     SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void     CopyData(T* data, VkDeviceSize size);
		VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void     Destroy();

	private:

	public:

		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		VkDeviceSize mSize = 0;
		VkDeviceSize mAlignment = 0;
		void* mapped = nullptr;

	private:
	};

	template<typename T>
	inline Buffer<T>::Buffer()
	{
	}

	template<typename T>
	inline Buffer<T>::~Buffer()
	{
		if (mBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(LOGICAL_DEVICE, mBuffer, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: VERTEX BUFFER");
		}

		if (mMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(LOGICAL_DEVICE, mMemory, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE FREED: VERTEX BUFFER MEMORY");
		}
	}

	template<typename T>
	inline void Buffer<T>::Create(const std::vector<T>& vertices, VkBufferUsageFlagBits flags)
	{
		mSize = sizeof(T) * vertices.size();

		// staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		MemoryUtility::CreateBuffer(mSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			ALLOCATION_CALLBACK, &stagingBuffer, &stagingBufferMemory);

		// map memory and copy data to staging buffer
		void* data;
		CHECK_RESULT(vkMapMemory(LOGICAL_DEVICE, stagingBufferMemory, 0, mSize, 0, &data), "VERTEX BUFFER: FAILED TO MAP VERTEX DATA TO VERTEX BUFFER MEMORY");
		memcpy(data, vertices.data(), (size_t)mSize);
		vkUnmapMemory(LOGICAL_DEVICE, stagingBufferMemory);

		// vertex buffer
		MemoryUtility::CreateBuffer(mSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			ALLOCATION_CALLBACK, &mBuffer, &mMemory);

		MemoryUtility::CopyBuffer(TRANSFER_QUEUE, TRANSFER_CMD_POOL, stagingBuffer, mBuffer, mSize);

		vkDestroyBuffer(LOGICAL_DEVICE, stagingBuffer, ALLOCATION_CALLBACK);
		RENDERER_DEBUG("RESOURCE DESTROYED: STAGING BUFFER");
		vkFreeMemory(LOGICAL_DEVICE, stagingBufferMemory, ALLOCATION_CALLBACK);
		RENDERER_DEBUG("RESOURCE FREED: STAGING BUFFER MEMORY");
	}

	template<typename T>
	const VkBuffer& Buffer<T>::GetVulkanBuffer()
	{
		return mBuffer;
	}
	template<typename T>
	inline VkResult Buffer<T>::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(LOGICAL_DEVICE, mMemory, offset, size, 0, &mapped);
	}

	template<typename T>
	inline void Buffer<T>::Unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(LOGICAL_DEVICE, mMemory);
			mapped = nullptr;
		}
	}

	template<typename T>
	inline VkResult Buffer<T>::Bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(LOGICAL_DEVICE, mBuffer, mMemory, offset);
	}

	template<typename T>
	inline void Buffer<T>::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = mBuffer;
		descriptor.range = size;
	}

	template<typename T>
	inline void Buffer<T>::CopyData(T* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	template<typename T>
	inline VkResult Buffer<T>::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = mMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(LOGICAL_DEVICE, 1, &mappedRange);
	}

	template<typename T>
	inline VkResult Buffer<T>::Invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = mMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
	}

	template<typename T>
	inline void Buffer<T>::Destroy()
	{
		if (mBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(LOGICAL_DEVICE, mBuffer, nullptr);
		}
		if (mMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(LOGICAL_DEVICE, mMemory, nullptr);
		}
	}
}
