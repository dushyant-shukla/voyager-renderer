#include "Texture.h"
#include "ResourceLoader.h"
#include "utility/MemoryUtility.h"
#include "utility/ImageUtility.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
		vkDestroyImage(mLogicalDevice, mImage, nullptr);
		vkFreeMemory(mLogicalDevice, mMemory, nullptr);
	}

	void Texture::Create(const char* filename, VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks, VkCommandPool transferCommandPool, VkQueue transferQueue)
	{
		mLogicalDevice = logicalDevice;
		mAllocationCallbacks = allocationCallbacks;

		int width, height;
		VkDeviceSize imageSize;

		stbi_uc* imageData = ResourceLoader::LoadTextureFile(std::string(filename), &width, &height, &imageSize);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		MemoryUtility::CreateBuffer(VK_NULL_HANDLE, logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mAllocationCallbacks,
			&stagingBuffer, &stagingMemory);

		void* data;
		vkMapMemory(mLogicalDevice, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, imageData, static_cast<unsigned int>(imageSize));
		vkUnmapMemory(mLogicalDevice, stagingMemory);

		stbi_image_free(imageData);

		ImageUtility::CreateImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mMemory);

		// transition image to be DST for copy operation
		ImageUtility::TransitionImageLayout(transferQueue, transferCommandPool, mImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// copy data to image
		MemoryUtility::CopyBufferToImage(transferQueue, transferCommandPool,
			stagingBuffer, mImage, static_cast<unsigned int> (width), static_cast<unsigned int> (height));

		// transition image to shader readable for shader usage
		ImageUtility::TransitionImageLayout(transferQueue, transferCommandPool, mImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingMemory, nullptr);

		RENDERER_DEBUG("RESOURCE CREATED: IMAGE TEXTURE (" + std::string(filename) + ")");
	}
}