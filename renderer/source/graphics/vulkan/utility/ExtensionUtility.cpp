#include "ExtensionUtility.h"

#include <GLFW/glfw3.h>

namespace vr
{
	/*
	Returns true if support for required validation layer is available.
	*/
	bool CheckValidationLayerSupport()
	{
		unsigned int count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);

		std::vector<VkLayerProperties> properties(count);
		vkEnumerateInstanceLayerProperties(&count, properties.data());

		for (const char* layer : VALIDATION_LAYERS)
		{
			for (const auto& property : properties)
			{
				if (strcmp(layer, property.layerName) == 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	std::vector<const char*> GetRequiredExtensions()
	{
		unsigned int glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensions;
	}
}