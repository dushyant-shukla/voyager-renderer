#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "utility/RendererCoreUtility.h"
#include "utility/MemoryUtility.h"

namespace vr
{
	template <typename T>
	class Buffer
	{
	public:

		Buffer();
		~Buffer();

		void Create(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkQueue& transferQueue, const VkCommandPool& commandPool, VkAllocationCallbacks* allocationCallbacks, const std::vector<T>& vertices, VkBufferUsageFlagBits flags);

		const VkBuffer& GetVulkanBuffer();

	private:

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

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
		vkDestroyBuffer(mLogicalDevice, mBuffer, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: VERTEX BUFFER");

		vkFreeMemory(mLogicalDevice, mMemory, nullptr);
		RENDERER_DEBUG("RESOURCE FREED: VERTEX BUFFER MEMORY");
	}

	template<typename T>
	inline void Buffer<T>::Create(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkQueue& transferQueue, const VkCommandPool& commandPool, VkAllocationCallbacks* allocationCallbacks, const std::vector<T>& vertices, VkBufferUsageFlagBits flags)
	{
		mLogicalDevice = device;
		mAllocationCallbacks = allocationCallbacks;
		VkDeviceSize bufferSize = sizeof(T) * vertices.size();

		// staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		MemoryUtility::CreateBuffer(physicalDevice, mLogicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			mAllocationCallbacks, &stagingBuffer, &stagingBufferMemory);

		// map memory and copy data to staging buffer
		void* data;
		CHECK_RESULT(vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data), "VERTEX BUFFER: FAILED TO MAP VERTEX DATA TO VERTEX BUFFER MEMORY");
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		// vertex buffer
		MemoryUtility::CreateBuffer(physicalDevice, mLogicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mAllocationCallbacks, &mBuffer, &mMemory);

		MemoryUtility::CopyBuffer(mLogicalDevice, transferQueue, commandPool, stagingBuffer, mBuffer, bufferSize);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: STAGING BUFFER");
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE FREED: STAGING BUFFER MEMORY");
	}

	template<typename T>
	const VkBuffer& Buffer<T>::GetVulkanBuffer()
	{
		return mBuffer;
	}
}
