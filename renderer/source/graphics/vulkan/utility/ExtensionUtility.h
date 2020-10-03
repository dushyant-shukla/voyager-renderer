#pragma once

#include "RendererCoreUtility.h"
#include <array>
#include <vector>

namespace vr
{
	class ExtensionUtility
	{
	public:

		inline static const char* VALIDATION_LAYERS[] = { "VK_LAYER_KHRONOS_validation" };
		inline static const char* DEVICE_EXTENSIONS[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		inline static const char* INSTANCE_EXTENSIONS[] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

		static bool CheckValidationLayerSupport();
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckInstanceExtentionSupport(std::vector<const char*>* checkExtensions);
		static bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device);
	};
}
