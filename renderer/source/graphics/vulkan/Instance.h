#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include <string>

namespace vr
{
	class Instance
	{
	public:

		static Instance* CreateInstance(const std::string& appName);

		~Instance();

		const VkInstance& GetVulkanInstance();

	private:

		Instance();

	private:

		std::string mAppName;
		VkInstance mInstance;
		VkDebugUtilsMessengerEXT mDebugMessenger;
	};
}
