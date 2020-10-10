#include "DepthBuffer.h"
#include "utility/RendererCoreUtility.h"
#include "assertions.h"
#include "utility/MemoryUtility.h"
#include "RendererState.h"

namespace vr
{
	DepthBuffer::DepthBuffer() :
		mFormat(),
		mImage(),
		mImageView(),
		mMemory()
	{
	}

	DepthBuffer::~DepthBuffer()
	{
		if (mImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(LOGICAL_DEVICE, mImageView, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: DEPTH BUFFER IMAGE VIEW");
		}

		if (mImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(LOGICAL_DEVICE, mImage, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: DEPTH BUFFER IMAGE");
		}

		if (mMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(LOGICAL_DEVICE, mMemory, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE FREED: DEPTH BUFFER MEMORY");
		}
	}

	void DepthBuffer::CreateDefault(const VkExtent2D& swapchainExtent)
	{
		ChooseSupportedFormat();

		// create image
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = swapchainExtent.width;
		imageCreateInfo.extent.height = swapchainExtent.height;
		imageCreateInfo.extent.depth = 1;	// depth of image (just 1, no 3d aspect)
		imageCreateInfo.mipLevels = 1;		// number of mip map levels
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = mFormat;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;	// how image data should be tiled (arranged for optimal reading)
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// layout of image data on creation
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;	// bit flags defining what images will be used for
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;	// number of samples for multi-sampling
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	// whether the image can be shared between queues

		CHECK_RESULT(vkCreateImage(LOGICAL_DEVICE, &imageCreateInfo, ALLOCATION_CALLBACK, &mImage), "RESOURCE CREATION FAILED: DEPTH BUFFER IMAGE");
		RENDERER_DEBUG("RESOURCE CREATED: DEPTH BUFFER IMAGE");

		// create memory for the image
		// get memory requirements for a type of image
		VkMemoryRequirements memRequirements = {};
		vkGetImageMemoryRequirements(LOGICAL_DEVICE, mImage, &memRequirements);

		// allocate memory using image requirements and user defined properties
		VkMemoryAllocateInfo memAllocateInfo = {};
		memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocateInfo.allocationSize = memRequirements.size;
		memAllocateInfo.memoryTypeIndex = MemoryUtility::FindMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		CHECK_RESULT(vkAllocateMemory(LOGICAL_DEVICE, &memAllocateInfo, ALLOCATION_CALLBACK, &mMemory), "RESOURCE ALLOCATION FAILED: DEPTH BUFFER MEMORY");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DEPTH BUFFER MEMORY");

		// connect allocated memory to image
		CHECK_RESULT(vkBindImageMemory(LOGICAL_DEVICE, mImage, mMemory, 0), "DEPTH BUFFER: FAILED TO BIND ALLOCATED MEMORY TO IMAGE");
		RENDERER_DEBUG("DEPTH BUFFER: SUCCESSFULLY BOUND IMAGE TO ALLOCATED MEMORY");

		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mImage;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;				// Allows remapping of RGBA components to other RGBA values
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Sub-resources allow the view to view only a part of an image
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;	// which aspect of image to view (eg. COLOR_BIT for viewing color)
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;							// start mipmap level to view from
		imageViewCreateInfo.subresourceRange.levelCount = 1;							// number of mipmap levels to view
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;						// start array level to view from
		imageViewCreateInfo.subresourceRange.layerCount = 1;							// number of array levels to view

		// create an image view
		CHECK_RESULT(vkCreateImageView(LOGICAL_DEVICE, &imageViewCreateInfo, ALLOCATION_CALLBACK, &mImageView), "RESOURCE CREATION FAILED: DEPTH BUFFER IMAGE VIEW");
		RENDERER_DEBUG("RESOURCE CREATED: DEPTH BUFFER IMAGE VIEW");
	}

	const VkImageView& DepthBuffer::GetImageView()
	{
		return mImageView;
	}

	const VkFormat& DepthBuffer::GetFormat()
	{
		return mFormat;
	}

	void DepthBuffer::ChooseSupportedFormat()
	{
		const std::vector<VkFormat>& formats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };
		const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		const VkFormatFeatureFlags featureFlags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

		//loop through options and find a compatible one
		for (size_t i = 0; i < formats.size(); ++i)
		{
			// get properties for a given format on this device
			VkFormatProperties properties = {};
			vkGetPhysicalDeviceFormatProperties(PHYSICAL_DEVICE, formats[i], &properties);

			// depending on tiling choice, need to check for different bit flag
			if (tiling == VK_IMAGE_TILING_LINEAR && ((properties.linearTilingFeatures & featureFlags) == featureFlags))
			{
				mFormat = formats[i];
				return;
			}
			if (tiling == VK_IMAGE_TILING_OPTIMAL && ((properties.optimalTilingFeatures & featureFlags) == featureFlags))
			{
				mFormat = formats[i];
				return;
			}
		}

		THROW("FAILED TO FIND A SUITABLE FORMAT FOR DEPTH BUFFER");
	}
}