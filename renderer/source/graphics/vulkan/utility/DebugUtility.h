#pragma once

#include "RendererCoreUtility.h"

namespace vr
{
	class DebugUtility
	{
	public:

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		static 	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static void CreateDebugMessenger(VkDebugUtilsMessengerEXT& const messenger, VkInstance& const instance, const VkDebugUtilsMessengerCreateInfoEXT& info, VkAllocationCallbacks const* const pAllocator);
		static void DestroyDebugMessenger(VkDebugUtilsMessengerEXT& const messenger, VkInstance& const instance, VkAllocationCallbacks const* const pAllocator);
	};
}
