#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Instance.h"

namespace vr
{
	//class Instance;

	class Surface
	{
	public:

		static Surface* Surface::CreateWindowSurface(Instance* const instance, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks);

		~Surface();

		inline VkSurfaceKHR GetVulkanSurface() { return mSurface; }

	private:

		Surface(Instance* const instance, VkAllocationCallbacks* const allocationCallbacks);

	private:
		Instance* const mInstance;

		VkSurfaceKHR mSurface;
		VkAllocationCallbacks* const mAllocationCallbacks;
	};
}
