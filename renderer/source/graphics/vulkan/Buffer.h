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

		void Create(const VkQueue& transferQueue, const VkCommandPool& commandPool, const std::vector<T>& vertices, VkBufferUsageFlagBits flags);

		const VkBuffer& GetVulkanBuffer();

	private:

	private:

		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
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
	inline void Buffer<T>::Create(const VkQueue& transferQueue, const VkCommandPool& commandPool, const std::vector<T>& vertices, VkBufferUsageFlagBits flags)
	{
		VkDeviceSize bufferSize = sizeof(T) * vertices.size();

		// staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		MemoryUtility::CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			ALLOCATION_CALLBACK, &stagingBuffer, &stagingBufferMemory);

		// map memory and copy data to staging buffer
		void* data;
		CHECK_RESULT(vkMapMemory(LOGICAL_DEVICE, stagingBufferMemory, 0, bufferSize, 0, &data), "VERTEX BUFFER: FAILED TO MAP VERTEX DATA TO VERTEX BUFFER MEMORY");
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(LOGICAL_DEVICE, stagingBufferMemory);

		// vertex buffer
		MemoryUtility::CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			ALLOCATION_CALLBACK, &mBuffer, &mMemory);

		MemoryUtility::CopyBuffer(transferQueue, commandPool, stagingBuffer, mBuffer, bufferSize);

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
}
