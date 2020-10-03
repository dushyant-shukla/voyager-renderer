#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class Window;
	class Instance;

	class Surface
	{
		static Surface* CreateWindowSurface(Instance const* const instance, Window const* const window, VkAllocationCallbacks const* allocationCallbacks);

		~Surface();

	private:

		Surface(Instance* const instance, Window* const window, VkAllocationCallbacks* allocationCallbacks);

		Instance* mInstance;
		VkSurfaceKHR mSurface;
		Window* mWindow;
		VkAllocationCallbacks* mAllocationCallbacks;
	};
}
