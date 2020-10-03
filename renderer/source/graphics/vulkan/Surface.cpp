#include "Surface.h"
#include "utility/RendererCoreUtility.h"
#include "window/Window.h"
#include "Instance.h"

#include <GLFW/glfw3.h>

namespace vr
{
	Surface* vr::Surface::CreateWindowSurface(Instance const* const instance, Window const* const window, VkAllocationCallbacks const* allocationCallbacks)
	{
		return nullptr;
	}

	vr::Surface::Surface(Instance* const instance, Window* const window, VkAllocationCallbacks* allocationCallbacks)
		: mInstance(instance), mWindow(window), mAllocationCallbacks(allocationCallbacks)
	{
		CHECK_RESULT(glfwCreateWindowSurface(mInstance->GetVulkanInstance(), mWindow->GetNativeWindow(), nullptr, &mSurface), "Failed to create window surface.");
	}

	vr::Surface::~Surface()
	{
	}
}