#include "Texture.h"
#include "ResourceLoader.h"
#include "utility/MemoryUtility.h"
#include "utility/ImageUtility.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"
#include "Model.h"

namespace vr
{
	Texture::Texture() {}

	Texture::Texture(unsigned int binding) : mBinding(binding)
	{
	}

	Texture::~Texture()
	{
		if (mImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(LOGICAL_DEVICE, mImage, nullptr);
			RENDERER_DEBUG("RESOURCE DESTROYED: TEXTURE IMAGE");
		}

		if (mImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(LOGICAL_DEVICE, mImageView, nullptr);
			RENDERER_DEBUG("RESOURCE DESTROYED: TEXTURE IMAGE VIEW");
		}

		if (mMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(LOGICAL_DEVICE, mMemory, nullptr);
			RENDERER_DEBUG("RESOURCE FREED: TEXTURE IMAGE MEMORY");
		}
	}

	void Texture::LoadFromFile(const char* filename)
	{
		int width, height;
		VkDeviceSize imageSize;

		stbi_uc* imageData = ResourceLoader::LoadTextureFile(std::string(filename), &width, &height, &imageSize);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		MemoryUtility::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ALLOCATION_CALLBACK,
			&stagingBuffer, &stagingMemory);

		void* data;
		vkMapMemory(LOGICAL_DEVICE, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, imageData, static_cast<unsigned int>(imageSize));
		vkUnmapMemory(LOGICAL_DEVICE, stagingMemory);

		stbi_image_free(imageData);

		ImageUtility::CreateImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mMemory);

		// transition image to be DST for copy operation
		ImageUtility::TransitionImageLayout(TRANSFER_QUEUE, TRANSFER_CMD_POOL, mImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// copy data to image
		MemoryUtility::CopyBufferToImage(TRANSFER_QUEUE, TRANSFER_CMD_POOL,
			stagingBuffer, mImage, static_cast<unsigned int> (width), static_cast<unsigned int> (height));

		// transition image to shader readable for shader usage
		ImageUtility::TransitionImageLayout(TRANSFER_QUEUE, TRANSFER_CMD_POOL, mImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		ImageUtility::CreateImageView(mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mImageView);

		vkDestroyBuffer(LOGICAL_DEVICE, stagingBuffer, nullptr);
		vkFreeMemory(LOGICAL_DEVICE, stagingMemory, nullptr);

		RENDERER_DEBUG("RESOURCE CREATED: IMAGE TEXTURE (" + std::string(filename) + ")");
	}

	void Texture::LoadFromFile(const char* filename, const VkSampler& sampler)
	{
		LoadFromFile(filename);

		mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mImageInfo.imageView = mImageView;
		mImageInfo.sampler = sampler;
	}

	void Texture::LoadWithData(const unsigned char* data, const VkDeviceSize size, const int width, const int height, const VkSampler& sampler)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		MemoryUtility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ALLOCATION_CALLBACK,
			&stagingBuffer, &stagingMemory);

		void* handle;
		vkMapMemory(LOGICAL_DEVICE, stagingMemory, 0, size, 0, &handle);
		memcpy(handle, data, static_cast<unsigned int>(size));
		vkUnmapMemory(LOGICAL_DEVICE, stagingMemory);

		ImageUtility::CreateImage(width, height,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mImage, mMemory);

		// transition image to be DST for copy operation
		ImageUtility::TransitionImageLayout(TRANSFER_QUEUE, TRANSFER_CMD_POOL, mImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// copy data to image
		MemoryUtility::CopyBufferToImage(TRANSFER_QUEUE, TRANSFER_CMD_POOL,
			stagingBuffer, mImage, static_cast<unsigned int> (width), static_cast<unsigned int> (height));

		// transition image to shader readable for shader usage
		ImageUtility::TransitionImageLayout(TRANSFER_QUEUE, TRANSFER_CMD_POOL, mImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		ImageUtility::CreateImageView(mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mImageView);

		vkDestroyBuffer(LOGICAL_DEVICE, stagingBuffer, nullptr);
		vkFreeMemory(LOGICAL_DEVICE, stagingMemory, nullptr);

		mImageInfo = {};
		mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mImageInfo.imageView = mImageView;
		mImageInfo.sampler = sampler;
	}

	const VkImageView& Texture::GetVulkanImageView()
	{
		return mImageView;
	}

	const VkImage& Texture::GetVulkanImage()
	{
		return mImage;
	}
}