#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "Device.h"
#include "Surface.h"

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

	private:

		Swapchain(Device* const device, Surface* const surface, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks);

		const VkSurfaceFormatKHR& ChooseSurfaceFormat();
		const VkPresentModeKHR& ChoosePresentationMode();

		/*
			The swap extent is the resolution of the swap chain images and it's almost
			always exactly equal to the resolution of the window that we're drawing to.
		*/
		VkExtent2D ChooseSwapExtent();

		void PopulateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectFlags, VkImageView& imageView);

	private:

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