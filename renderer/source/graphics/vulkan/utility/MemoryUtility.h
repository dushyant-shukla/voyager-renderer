#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class MemoryUtiity
	{
	public:

		static unsigned int FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int allowedTypes, VkMemoryPropertyFlags propertyFlags);

		static void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize bufferSize,
			VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferProperties,
			VkAllocationCallbacks* allocationCallbacks, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

		static VkCommandBuffer BeginCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool);

		static void EndAndSubmitCommandBuffer(const VkDevice& logicalDevice, const VkCommandPool& commandPool, const VkQueue& queue, const VkCommandBuffer& commandBuffer);

		static void CopyBuffer(const VkDevice& logicalDevice, const VkQueue& queue, const VkCommandPool& commandPool, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize bufferSize);
	};
}
