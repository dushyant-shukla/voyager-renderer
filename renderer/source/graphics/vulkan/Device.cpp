#include "Device.h"
#include "assertions.h"
#include "utility/ExtensionUtility.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <iostream>

namespace vr
{
	Device* Device::InitializeDevice(Instance* const instance, Surface* const surface)
	{
		static Device device(instance, surface);
		device.SetupPhysicalDevice();
		/*device.SetupLogicalDevice();*/
		return &device;
	}

	Device::Device(Instance* instance, Surface* surface) : mInstance(instance), mSurface(surface)
	{}

	Device::~Device()
	{
		if (mLogicalDevice.device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(mLogicalDevice.device, nullptr);
			RENDERER_DEBUG("RESOURCE DESTROYED: LOGICAL DEVICE");
		}
	}

	void Device::SetupPhysicalDevice()
	{
		unsigned int count = 0;
		vkEnumeratePhysicalDevices(mInstance->GetVulkanInstance(), &count, nullptr);
		ASSERT_SUCCESS_AND_THROW(count >= 0, "RESOURCE ALLOCATION FAILED: PHYSICAL DEVICE: VULKAN IS NOT SUPPORTED ON AVAILABLE DEVICES");
		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(mInstance->GetVulkanInstance(), &count, devices.data());
		std::vector<PhysicalDevice> physicalDevices(devices.begin(), devices.end());
		auto itr = std::find_if(physicalDevices.begin(), physicalDevices.end(), [this](PhysicalDevice& device) {
			return IsDeviceSuitable(device);
			});
		if (itr != physicalDevices.end())
		{
			mPhysicalDevice = *itr;
			RENDERER_DEBUG("RESOURCE ALLOCATED: PHYSICAL DEVICE");
		}
		else
		{
			THROW("RESOURCE ALLOCATION FAILED: PHYSICAL DEVICE: NO SUITABLE DEVICES AVAILABLE");
		}
	}

	void Device::SetupLogicalDevice(const VkPhysicalDeviceFeatures& requiredFeatures)
	{
		// get the queue family indices for the chosen physical device
		QueueFamilyIndices indices = mPhysicalDevice.queueFamilies;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		//const std::set<unsigned int> queueFamilyIndices = { indices.graphics.value(), indices.presentation.value() };
		const std::set<unsigned int> queueFamilyIndices = { indices.graphics.value(), indices.compute.value(), indices.transfer.value(), indices.presentation.value() };
		for (int queueFamilyIndex : queueFamilyIndices)
		{
			// queues the logical device needs to create
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;					// index of family to create a queue from
			queueCreateInfo.queueCount = 1;											// number of queues to create
			float priority = 1;
			queueCreateInfo.pQueuePriorities = &priority;							// Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest priority)
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// information to create logical device
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<unsigned int>(queueCreateInfos.size());							// number of queues to create
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();														// list of queue create infos so device can create required queues
		deviceCreateInfo.enabledExtensionCount = static_cast<unsigned int>(std::size(ExtensionUtility::DEVICE_EXTENSIONS));	// number of enabled logical device extensions
		deviceCreateInfo.ppEnabledExtensionNames = ExtensionUtility::DEVICE_EXTENSIONS;										// list of enabled logical device extensions
		deviceCreateInfo.pEnabledFeatures = &requiredFeatures;

		// create the logical device for the given physical device
		CHECK_RESULT(vkCreateDevice(mPhysicalDevice.device, &deviceCreateInfo, nullptr, &mLogicalDevice.device), "RESOURCE CREATION FAILED: LOGICAL DEVICE");
		RENDERER_DEBUG("RESOURCE CREATED: LOGICAL DEVICE");

		// Queues are created at the same time as logical device
		// From given logical device, of given queue family, of given Queue Index (0 since only one queue), place reference in given VkQueue
		vkGetDeviceQueue(mLogicalDevice.device, indices.graphics.value(), 0, &mLogicalDevice.graphicsQueue);
		vkGetDeviceQueue(mLogicalDevice.device, indices.presentation.value(), 0, &mLogicalDevice.presentationQueue);
		vkGetDeviceQueue(mLogicalDevice.device, indices.compute.value(), 0, &mLogicalDevice.computeQueue);
		vkGetDeviceQueue(mLogicalDevice.device, indices.transfer.value(), 0, &mLogicalDevice.transferQueue);
	}

	bool Device::IsDeviceSuitable(PhysicalDevice& device)
	{
		//// information about the device itself (Id, name, type, vendor, etc)
		vkGetPhysicalDeviceProperties(device.device, &device.properties);

		//// information about what the device can do (geo shader, tess shader, wide lines etc)
		vkGetPhysicalDeviceFeatures(device.device, &device.features);

		QueryDeviceQueueFamilies(device);

		bool extensionsSupported = ExtensionUtility::CheckDeviceExtensionSupport(device.device);
		bool swapchainValid = false;
		if (extensionsSupported)
		{
			UpdateSwapChainDetails(device);
			swapchainValid = !device.swapChainDetails.presentationModes.empty() && !device.swapChainDetails.formats.empty();
		}

		return device.queueFamilies.IsValid() &&
			extensionsSupported && swapchainValid;
	}

	void Device::QueryDeviceQueueFamilies(PhysicalDevice& device)
	{
		unsigned int queuefamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device.device, &queuefamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queuefamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device.device, &queuefamilyCount, queueFamilies.data());

		for (size_t i = 0; i < queueFamilies.size(); ++i)
		{
			const auto& queueFamily = queueFamilies[i];
			// first check if the queue family has at least one queue (could have no queues)
			// queue could be multiple types defined through bit-field, need to bitwise AND with VK_QUEUE_*_BIT to check if it has required type
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				device.queueFamilies.graphics = i; // if queue family is valid,  then get index
			}
			else if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				device.queueFamilies.compute = i; // if queue family is valid,  then get index
			}
			else if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				device.queueFamilies.transfer = i; // if queue family is valid,  then get index
			}

			// check if queue family supports presentation
			VkBool32 presentationSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device.device, i, mSurface->GetVulkanSurface(), &presentationSupport);
			if (queueFamily.queueCount > 0 && presentationSupport)
			{
				// graphics queue can also be presentation queue
				device.queueFamilies.presentation = i;
			}

			// Check if queue family indices are in valid states, then stop searching
			if (device.queueFamilies.IsValid())
			{
				break;
			}
		}

		/*
			If no exclusive compute and transfer queues could be found, initialize with graphics queue
		*/
		if (!device.queueFamilies.compute.has_value())
		{
			device.queueFamilies.compute = device.queueFamilies.graphics;
		}

		if (!device.queueFamilies.transfer.has_value())
		{
			device.queueFamilies.transfer = device.queueFamilies.graphics;
		}
	}

	void Device::UpdateSwapChainDetails(PhysicalDevice& device)
	{
		VkSurfaceKHR surface = mSurface->GetVulkanSurface();

		// Get surface capabilities for the given surface on the given physical device
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.device, surface, &device.swapChainDetails.surfaceCapabilities);

		// Formats
		unsigned int formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.device, surface, &formatCount, nullptr);
		if (formatCount > 0)
		{
			device.swapChainDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device.device, surface, &formatCount, device.swapChainDetails.formats.data());
		}

		// Presentation modes;
		unsigned int presentationCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.device, surface, &presentationCount, nullptr);
		if (presentationCount > 0)
		{
			device.swapChainDetails.presentationModes.resize(presentationCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device.device, surface, &presentationCount, device.swapChainDetails.presentationModes.data());
		}
	}
}