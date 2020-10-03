#include "Instance.h"
#include "utility/ExtensionUtility.h"
#include "utility/DebugUtility.h"
#include "assertions.h"

namespace vr
{
	Instance* Instance::CreateInstance(const std::string& appName)
	{
		static Instance instance;

#ifdef ENABLE_VALIDATION
		ASSERT_SUCCESS_AND_THROW(ExtensionUtility::CheckValidationLayerSupport(), "Validation layers requested, but not available.");
#endif

		// Information about application
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Voyager Renderer";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		std::vector<const char*> extensions = ExtensionUtility::GetRequiredExtensions();
		ASSERT_SUCCESS_AND_THROW(ExtensionUtility::CheckInstanceExtentionSupport(&extensions), "Required extensions not available.");

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		DebugUtility::PopulateDebugMessengerCreateInfo(debugCreateInfo);
#ifdef ENABLE_VALIDATION
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ExtensionUtility::VALIDATION_LAYERS.size());
		instanceCreateInfo.ppEnabledLayerNames = ExtensionUtility::VALIDATION_LAYERS.data();
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.ppEnabledLayerNames = nullptr;
		instanceCreateInfo.pNext = nullptr;
#endif

		CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance.mInstance), "Failed to create a Vulkan instance.");
		RENDERER_DEBUG("Vulkan instance created.");

#ifdef ENABLE_VALIDATION
		DebugUtility::CreateDebugMessenger(instance.mDebugMessenger, instance.mInstance, debugCreateInfo, nullptr);
#endif

		return &instance;
	}

	Instance::Instance() {}

	Instance::~Instance()
	{
		DebugUtility::DestroyDebugMessenger(mDebugMessenger, mInstance, nullptr);
		vkDestroyInstance(mInstance, nullptr);
		RENDERER_DEBUG("Vulkan instance destroyed.");
	}

	const VkInstance& Instance::GetVulkanInstance()
	{
		return mInstance;
	}
}