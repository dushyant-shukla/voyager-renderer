#include "MemoryUtility.h"
#include "RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	unsigned int MemoryUtility::FindMemoryTypeIndex(unsigned int allowedTypes, VkMemoryPropertyFlags propertyFlags)
	{
		// Get properties of the physical device memory
		VkPhysicalDeviceMemoryProperties memoryProperties = {};
		vkGetPhysicalDeviceMemoryProperties(PHYSICAL_DEVICE, &memoryProperties);

		for (unsigned int i = 0; i < memoryProperties.memoryTypeCount; ++i)
		{
			// index of memory types must match corresponding bit in allowed types
			// desired property bit flags are part of memory type's property flags
			if ((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags))
			{
				return i;	// this memory type is valid so return its index
			}
		}
	}

	void MemoryUtility::CreateBuffer(VkDeviceSize bufferSize,
		VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferProperties, VkAllocationCallbacks* allocationCallbacks, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
	{
		// Information to create buffer (doesn't include assigning the memory)
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;								// size of buffer (sizeof 1 vertx * number of vertices)
		bufferInfo.usage = bufferUsageFlags;						// multiple types of buffers possible, we need vertex buffer
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// similar to swapchain, images can share vertex buffers

		CHECK_RESULT(vkCreateBuffer(LOGICAL_DEVICE, &bufferInfo, ALLOCATION_CALLBACK, buffer), "RESOURCE CREATION FAILED: VERTEX BUFFER");

		// get buffer memory requirements
		VkMemoryRequirements memRequirements = {};
		vkGetBufferMemoryRequirements(LOGICAL_DEVICE, *buffer, &memRequirements);

		// allocate memory to buffer
		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memRequirements.size;			// Index of memory type on physical device that has required bit flags
		memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, bufferProperties);
		// Allocate memory to VkDeviceMemory
		CHECK_RESULT(vkAllocateMemory(LOGICAL_DEVICE, &memoryAllocateInfo, ALLOCATION_CALLBACK, bufferMemory), "RESOURCE ALLOCATION FAILED: VERTEX BUFFER MEMORY");
		// Bind memory to vertex buffer
		CHECK_RESULT(vkBindBufferMemory(LOGICAL_DEVICE, *buffer, *bufferMemory, 0), "VERTEX BUFFER MEMORY FAILED TO BE BOUND TO VERTEX BUFFER");
	}

	VkCommandBuffer MemoryUtility::BeginCommandBuffer(VkCommandPool commandPool)
	{
		VkCommandBuffer commandBuffer;

		// command buffer details
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		// allocate command buffer from pool
		vkAllocateCommandBuffers(LOGICAL_DEVICE, &allocInfo, &commandBuffer);

		// Information to being the command buffer record
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// we are using the command buffer once, so setup for one time submit

		// Being recording transfer commands
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void MemoryUtility::EndAndSubmitCommandBuffer(const VkCommandPool& commandPool, const VkQueue& queue, const VkCommandBuffer& commandBuffer)
	{
		// TODO: Checkout VulkanDevice.cpp line: 508 vulkan-examples for flush command buffer

		// end commands
		vkEndCommandBuffer(commandBuffer);

		// execute the command buffer
		// Queue submission information
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// submit transfer command to transfer queue and wait until it finished
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		// free command buffer
		vkFreeCommandBuffers(LOGICAL_DEVICE, commandPool, 1, &commandBuffer);
	}

	void MemoryUtility::CopyBuffer(const VkQueue& queue, const VkCommandPool& commandPool, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize bufferSize)
	{
		//create the buffer
		VkCommandBuffer transferCommandBuffer = BeginCommandBuffer(commandPool);

		// Region of data to copy from and to
		VkBufferCopy bufferCopyRegion = {};
		bufferCopyRegion.srcOffset = 0;
		bufferCopyRegion.dstOffset = 0;
		bufferCopyRegion.size = bufferSize;

		// command to copy src buffer to dst buffer
		vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

		EndAndSubmitCommandBuffer(commandPool, queue, transferCommandBuffer);
	}

	void MemoryUtility::CopyBufferToImage(const VkQueue transferQueue, const VkCommandPool& transferCommandPool, const VkBuffer& srcBuffer, VkImage& dstImage, const unsigned int& width, const unsigned int& height)
	{
		//create the buffer
		VkCommandBuffer transferCommandBuffer = BeginCommandBuffer(transferCommandPool);

		VkBufferImageCopy imageRegion = {};
		imageRegion.bufferOffset = 0;											// offset into data
		imageRegion.bufferRowLength = 0;										// offset row length to calculate data spacing
		imageRegion.bufferImageHeight = 0;										// image height to calculate data spacing
		imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// which aspect of image to copy
		imageRegion.imageSubresource.mipLevel = 0;								// mip map level to copy
		imageRegion.imageSubresource.baseArrayLayer = 0;						// starting array layer
		imageRegion.imageSubresource.layerCount = 1;							// number of layers to copy starting at base array layer
		imageRegion.imageOffset = { 0, 0, 0 };									// offset into image as opposed to raw data in buffer offset
		imageRegion.imageExtent = { width, height, 1 };							// size of region to copy as x, y, z values

		// copy buffer to given image
		vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

		EndAndSubmitCommandBuffer(transferCommandPool, transferQueue, transferCommandBuffer);
	}
}