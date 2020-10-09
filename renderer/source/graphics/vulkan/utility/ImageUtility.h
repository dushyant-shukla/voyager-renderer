#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class ImageUtility
	{
	public:

		static void CreateImage(const unsigned int& width, const unsigned int& height, const VkFormat& format, const VkImageTiling& tiling,
			const VkImageUsageFlags usageFlags, const VkMemoryPropertyFlags memoryFlags,
			VkImage& image, VkDeviceMemory& memory);

		static void TransitionImageLayout(const VkQueue& queue, const VkCommandPool commandPool, VkImage& image, VkFormat format, const VkImageLayout& oldLayout, const VkImageLayout& newLayout);

	public:

		static VkDevice logicalDevice;
	};
}
