#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class MemoryUtility
	{
	public:

		static unsigned int FindMemoryTypeIndex(unsigned int allowedTypes, VkMemoryPropertyFlags propertyFlags);

		static void CreateBuffer(VkDeviceSize bufferSize,
			VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferProperties,
			VkAllocationCallbacks* allocationCallbacks, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

		static VkCommandBuffer BeginCommandBuffer(VkCommandPool commandPool);

		static void EndAndSubmitCommandBuffer(const VkCommandPool& commandPool, const VkQueue& queue, const VkCommandBuffer& commandBuffer);

		static void CopyBuffer(const VkQueue& queue, const VkCommandPool& commandPool, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize bufferSize);

		static void CopyBufferToImage(const VkQueue transferQueue, const VkCommandPool& transferCommandPool, const VkBuffer& srcBuffer, VkImage& dstImage, const unsigned int& width, const unsigned int& height);
	};
}
