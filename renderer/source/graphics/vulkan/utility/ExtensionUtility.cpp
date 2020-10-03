#include "ExtensionUtility.h"

#include <GLFW/glfw3.h>

namespace vr
{
	std::vector<const char*> ExtensionUtility::VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

	/*
	Returns true if support for required validation layer is available.
	*/
	bool ExtensionUtility::CheckValidationLayerSupport()
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

	std::vector<const char*> ExtensionUtility::GetRequiredExtensions()
	{
		unsigned int glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensions;
	}

	bool ExtensionUtility::CheckInstanceExtentionSupport(std::vector<const char*>* checkExtensions)
	{
		unsigned int extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// check if given extensions are in the list of available extensions
		for (const auto& checkExtension : *checkExtensions)
		{
			bool hasExtension = false;
			for (const auto& extension : extensions)
			{
				if (strcmp(checkExtension, extension.extensionName) == 0)
				{
					hasExtension = true;
					break;
				}
			}

			if (!hasExtension)
			{
				return false;
			}
		}

		return true;
	}
}