#include "Surface.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	Surface* Surface::CreateWindowSurface(Instance* const instance, GLFWwindow* window, VkAllocationCallbacks* const allocationCallbacks)
	{
		static Surface surface(instance, allocationCallbacks);
		CHECK_RESULT(glfwCreateWindowSurface(surface.mInstance->GetVulkanInstance(), window, allocationCallbacks, &surface.mSurface), "Failed to create window surface.");
		RENDERER_DEBUG("RESOURCE CREATED: WINDOW SURFACE");
		return &surface;
	}

	Surface::Surface(Instance* const instance, VkAllocationCallbacks* const allocationCallbacks)
		: mInstance(instance), mAllocationCallbacks(allocationCallbacks)
	{
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(mInstance->GetVulkanInstance(), mSurface, nullptr);
		RENDERER_DEBUG("RESOURCE DESTROYED: WINDOW SURFACE");
	}
}