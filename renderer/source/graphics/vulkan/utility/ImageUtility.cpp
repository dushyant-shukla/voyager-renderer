#include "ImageUtility.h"
#include "RendererCoreUtility.h"
#include "MemoryUtility.h"

namespace vr
{
	VkDevice ImageUtility::logicalDevice = VK_NULL_HANDLE;

	void ImageUtility::CreateImage(const unsigned int& width, const unsigned int& height, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags usageFlags, const VkMemoryPropertyFlags memoryFlags, VkImage& image, VkDeviceMemory& memory)
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;	// depth of image (just 1, no 3d aspect)
		imageCreateInfo.mipLevels = 1;		// number of mip map levels
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = tiling;	// how image data should be tiled (arranged for optimal reading)
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// layout of image data on creation
		imageCreateInfo.usage = usageFlags;	// bit flags defining what images will be used for
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;	// number of samples for multi-sampling
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	// whether the image can be shared between queues

		CHECK_RESULT(vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image), "RESOURCE CREATION FAILED: IMAGE");

		VkMemoryRequirements memRequirements = {};
		vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo memAllocateInfo = {};
		memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocateInfo.allocationSize = memRequirements.size;
		memAllocateInfo.memoryTypeIndex = MemoryUtility::FindMemoryTypeIndex(VK_NULL_HANDLE, memRequirements.memoryTypeBits, memoryFlags);

		CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAllocateInfo, nullptr, &memory), "RESOURCE ALLOCATION FAILED: IMAGE MEMORY");

		// connect allocated memory to image
		CHECK_RESULT(vkBindImageMemory(logicalDevice, image, memory, 0), "FAILED TO BIND ALLOCATED MEMORY TO IMAGE!");
	}

	void ImageUtility::TransitionImageLayout(const VkQueue& queue, const VkCommandPool commandPool, VkImage& image, VkFormat format, const VkImageLayout& oldLayout, const VkImageLayout& newLayout)
	{
		VkCommandBuffer commandBuffer = MemoryUtility::BeginCommandBuffer(logicalDevice, commandPool);

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.newLayout = newLayout;									// layout to transition from
		imageMemoryBarrier.oldLayout = oldLayout;									// layout to transition to
		imageMemoryBarrier.image = image;											// image being accessed and modified as part of barrier
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// queue family to transition to
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// queue family to transition to
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// aspect of image being altered
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;						// first mip level to start alterations on
		imageMemoryBarrier.subresourceRange.levelCount = 1;							// number of mip levels to alter starting from base mip level
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;						// first layer to start alterations on
		imageMemoryBarrier.subresourceRange.layerCount = 1;							// number of layers to alter starting from base array layer

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		// if transitioning from new image to image ready to receive data
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = 0;								// memory access stage transition must happen after ...
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	// memory access stage transition must happen before ...

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// if transitioning from transfer destination to shader readable
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	// memory access stage transition must happen after ...
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;		// memory access stage transition must happen before ...

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier(commandBuffer,
			srcStage, dstStage,			// pipeline stages (match to src and dst access masks)
			0,							// dependency flags
			0, nullptr,					// memory barrier count and data
			0, nullptr,					// buffer memory barrier count and data
			1, &imageMemoryBarrier);	// image memory barrier count and data

		MemoryUtility::EndAndSubmitCommandBuffer(logicalDevice, commandPool, queue, commandBuffer);
	}
}