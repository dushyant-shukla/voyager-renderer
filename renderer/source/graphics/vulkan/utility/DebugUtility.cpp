#include "DebugUtility.h"

#include <vector>

namespace vr
{
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtility::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
#ifdef ENABLE_DEBUG_LOGGING
			RENDERER_WARN("INVALID OPERATION WARNING: {0}", pCallbackData->pMessage);
#endif
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			RENDERER_ERROR("INVALID OPERATION: {0}", pCallbackData->pMessage);
			break;
		};
		return VK_FALSE;
	}

	void DebugUtility::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
	{
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;
		info.pUserData = nullptr; // Optional
	}

	void DebugUtility::CreateDebugMessenger(VkDebugUtilsMessengerEXT& const messenger, VkInstance& const instance, const VkDebugUtilsMessengerCreateInfoEXT& info, VkAllocationCallbacks const* const pAllocator)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			CHECK_RESULT(func(instance, &info, pAllocator, &messenger), "Failed to create debug messenger.");
		}
		RENDERER_DEBUG("Debug messenger created.");
	}

	void DebugUtility::DestroyDebugMessenger(VkDebugUtilsMessengerEXT& const messenger, VkInstance& const instance, VkAllocationCallbacks const* const pAllocator)
	{
#ifdef ENABLE_VALIDATION
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, messenger, pAllocator);
		}
		RENDERER_DEBUG("Debug messenger destroyed.");
#endif
	}
}