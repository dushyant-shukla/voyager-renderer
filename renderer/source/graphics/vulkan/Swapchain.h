#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Device.h"
#include "Surface.h"

struct GLFWwindow;

namespace vr
{
	typedef struct SwapchainImage
	{
		VkImage image;
		VkImageView imageView;
	} SwapchainImage;

	class Swapchain
	{
	public:

		static Swapchain* CreateSwapchain(Device* const device, Surface* const surface, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks);

		~Swapchain();

		std::vector<SwapchainImage>& GetSwapchainImages();
		const VkExtent2D& GetSwapchainExtent();
		const VkSurfaceFormatKHR& GetSurfaceFormat();
		const VkSwapchainKHR& GetVulkanSwapChain();

	private:

		Swapchain(Device* const device, Surface* const surface, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks);

		const VkSurfaceFormatKHR& ChooseSurfaceFormat();
		const VkPresentModeKHR& ChoosePresentationMode();

		/*
			The swap extent is the resolution of the swap chain images and it's almost
			always exactly equal to the resolution of the window that we're drawing to.
		*/
		VkExtent2D ChooseSwapExtent();

	public:

		Surface* mSurface;
		Device* mDevice;
		GLFWwindow* mWindow;

		VkSurfaceFormatKHR mSurfaceFormat;
		VkPresentModeKHR mPresentationMode;
		VkExtent2D mExtent;
		VkSwapchainKHR mSwapchain;
		VkAllocationCallbacks* mAllocationCallbacks;
		std::vector<SwapchainImage> mImages;
	};
}