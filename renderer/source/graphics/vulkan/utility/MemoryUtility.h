#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class MemoryUtility
	{
	public:

		static unsigned int FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int allowedTypes, VkMemoryPropertyFlags propertyFlags);

		static void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize bufferSize,
			VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferProperties,
			VkAllocationCallbacks* allocationCallbacks, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

		static VkCommandBuffer BeginCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool);

		static void EndAndSubmitCommandBuffer(const VkDevice& logicalDevice, const VkCommandPool& commandPool, const VkQueue& queue, const VkCommandBuffer& commandBuffer);

		static void CopyBuffer(const VkDevice& logicalDevice, const VkQueue& queue, const VkCommandPool& commandPool, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize bufferSize);

		static void CopyBufferToImage(const VkQueue transferQueue, const VkCommandPool& transferCommandPool, const VkBuffer& srcBuffer, VkImage& dstImage, const unsigned int& width, const unsigned int& height);

	public:

		static VkPhysicalDevice PhysicalDevice;
		static VkDevice LogicalDevice;
		static VkAllocationCallbacks* AllocationCallbacks;
	};
}
