#include "SwapChain.h"
#include "utility/RendererCoreUtility.h"
#include "assertions.h"

namespace vr
{
	Swapchain* Swapchain::CreateSwapchain(Device* const device, Surface* const surface, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks)
	{
		static Swapchain swapchain(device, surface, window, allocationCallbacks);
		return &swapchain;
	}

	Swapchain::~Swapchain()
	{
		for (auto image : mImages)
		{
			vkDestroyImageView(mDevice->GetLogicalDevice().device, image.imageView, mAllocationCallbacks);
		}
		vkDestroySwapchainKHR(mDevice->GetLogicalDevice().device, mSwapchain, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: SWAPCHAIN");
	}

	std::vector<SwapchainImage>& Swapchain::GetSwapchainImages()
	{
		return mImages;
	}

	const VkExtent2D& Swapchain::GetSwapchainExtent()
	{
		return mExtent;
	}

	Swapchain::Swapchain(Device* const device, Surface* const surface, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks)
		: mDevice(device), mSurface(surface), mWindow(window), mAllocationCallbacks(allocationCallbacks), mSwapchain(VK_NULL_HANDLE)
	{
		SwapChainDetails swapchainDetails = mDevice->GetPhysicalDevice().swapChainDetails;

		mSurfaceFormat = ChooseSurfaceFormat();
		mPresentationMode = ChoosePresentationMode();
		mExtent = ChooseSwapExtent();

		// How many images are in the swap chain? Get 1 more than the minimum to allow for triple buffering
		unsigned int imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;
		// if max image count <= 0, then no limit
		if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 && swapchainDetails.surfaceCapabilities.maxImageCount < imageCount)
		{
			imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
		}

		// creation info for swap chain
		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.imageFormat = mSurfaceFormat.format;										// swapchain format
		swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;								// swapchain color space
		swapchainCreateInfo.presentMode = mPresentationMode;											// swapchain presentation mode
		swapchainCreateInfo.imageExtent = mExtent;														// swapchain image extent
		swapchainCreateInfo.minImageCount = imageCount;													// minimum images in swapchain
		swapchainCreateInfo.imageArrayLayers = 1;														// number of layers for each image in swapchain

		// Find the transformation of the surface
		if (swapchainDetails.surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			swapchainCreateInfo.preTransform = swapchainDetails.surfaceCapabilities.currentTransform;	// transform to perform on swapchain images
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;							// how to handle blending images with external graphics (eg. other windows)
		// Simply select the first composite alpha format available
		const std::vector<VkCompositeAlphaFlagBitsKHR> COMPOSITE_ALPHA_FLAGS = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};
		for (auto& compositeAlphaFlag : COMPOSITE_ALPHA_FLAGS) {
			if (swapchainDetails.surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
				swapchainCreateInfo.compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		swapchainCreateInfo.clipped = VK_TRUE;															// Whether to clip parts of images not in the view (eg. behind another window, off-screen, etc.)
		swapchainCreateInfo.surface = mSurface->GetVulkanSurface();										// swapchain surface

		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;							// how attachment images will be used
		// Enable transfer source on swap chain images if supported
		if (swapchainDetails.surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Enable transfer destination on swap chain images if supported
		if (swapchainDetails.surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		// Setting up queues
		// Graphics queues - draw on images
		// Presentation queues - take the images and present them on screen (surface)
		QueueFamilyIndices indices = mDevice->GetPhysicalDevice().queueFamilies;

		// If graphics and presentation families are different, then swapchain must let images be shared between families.
		if (indices.graphics.value() != indices.presentation.value())
		{
			unsigned int queueFamilyIndices[] = {
				(unsigned int)indices.graphics.value(),
				(unsigned int)indices.presentation.value()
			};

			// Concurrent mode requires you to specify in advance between which queue families ownership will be shared using the
			// queueFamilyIndexCount and pQueueFamilyIndices parameters
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// image share handling
			swapchainCreateInfo.queueFamilyIndexCount = 2;							// number of queues to sharing images between
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// array of queues to share between
		}
		else
		{
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		// If old swapchain been destroyed and this one replaces it, then link old one to quickly handover responsibilities
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		// Create swapchain
		CHECK_RESULT(vkCreateSwapchainKHR(mDevice->GetLogicalDevice().device, &swapchainCreateInfo, nullptr, &mSwapchain), "RESOURCE CREATION FAILED: SWAPCHAIN");
		RENDERER_DEBUG("RESOURCE CREATED: SWAPCHAIN");

		// Fetch swapchain images from swapchain after successful creation of swapchain
		unsigned int swapchainImageCount;
		CHECK_RESULT(vkGetSwapchainImagesKHR(mDevice->GetLogicalDevice().device, mSwapchain, &swapchainImageCount, nullptr), "UNABLE TO FETCH SWAPCHAIN IMAGES");
		std::vector<VkImage> images(swapchainImageCount);
		CHECK_RESULT(vkGetSwapchainImagesKHR(mDevice->GetLogicalDevice().device, mSwapchain, &swapchainImageCount, images.data()), "UNABLE TO FETCH SWAPCHAIN IMAGES");

		for (const auto& image : images)
		{
			SwapchainImage newImage = {};
			newImage.image = image;

			// Populate image view
			PopulateImageView(image, mSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, newImage.imageView);
			mImages.push_back(newImage);
		}
	}

	const VkSurfaceFormatKHR& Swapchain::ChooseSurfaceFormat()
	{
		for (const VkSurfaceFormatKHR& format : mDevice->GetPhysicalDevice().swapChainDetails.formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return mDevice->GetPhysicalDevice().swapChainDetails.formats[0];
	}

	const VkPresentModeKHR& Swapchain::ChoosePresentationMode()
	{
		for (const VkPresentModeKHR& mode : mDevice->GetPhysicalDevice().swapChainDetails.presentationModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::ChooseSwapExtent()
	{
		// Vulkan tells us to match the resolution of the window by setting the width and height in the currentExtent member.
		VkSurfaceCapabilitiesKHR surfaceCapabilities = mDevice->GetPhysicalDevice().swapChainDetails.surfaceCapabilities;
		if (surfaceCapabilities.currentExtent.width != std::numeric_limits<unsigned int>::max())
		{
			return surfaceCapabilities.currentExtent;
		}

		// However, some window managers do allow us to differ here and this is indicated by setting the width and height in
		// currentExtent to a special value: the maximum value of uint32_t. In that case we'll pick the resolution that best
		// matches the window within the minImageExtent and maxImageExtent bounds.
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);

		// Create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<unsigned int>(width);
		newExtent.height = static_cast<unsigned int>(height);

		// Surface also defines max and min, so make sure within boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));
		return newExtent;
	}

	void Swapchain::PopulateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectFlags, VkImageView& imageView)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;				// Allows remapping of RGBA components to other RGBA values
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Sub-resources allow the view to view only a part of an image
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;					// which aspect of image to view (eg. COLOR_BIT for viewing color)
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;							// start mipmap level to view from
		imageViewCreateInfo.subresourceRange.levelCount = 1;							// number of mipmap levels to view
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;						// start array level to view from
		imageViewCreateInfo.subresourceRange.layerCount = 1;							// number of array levels to view
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.pNext = NULL;

		// create an image view
		CHECK_RESULT(vkCreateImageView(mDevice->GetLogicalDevice().device, &imageViewCreateInfo, nullptr, &imageView), "RESOURCE CREATION FAILED: IMAGE VIEW");
	}
}