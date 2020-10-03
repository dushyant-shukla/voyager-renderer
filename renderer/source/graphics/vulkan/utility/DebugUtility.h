#pragma once

#include "RendererCoreUtility.h"

namespace vr
{
	//// DEBUG MESSENGER UTILITY FUNCTIONS ////
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkDebugUtilsMessengerEXT CreateDebugMessenger(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo, VkAllocationCallbacks const* const pAllocator);
	void DestroyDebugMessenger(const VkInstance& instance, const VkDebugUtilsMessengerEXT& debugMessenger, VkAllocationCallbacks const* const pAllocator);
}
