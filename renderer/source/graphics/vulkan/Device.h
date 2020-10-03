#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <iostream>

#include "Instance.h"
#include "Surface.h"
#include <ostream>

namespace vr
{
	typedef struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;		// surface properties, eg. image size/extent
		std::vector<VkSurfaceFormatKHR> formats;			// surface image formats, eg. RGBA and size of each channel
		std::vector<VkPresentModeKHR> presentationModes;	// 4 types of presentation modes: how images should be presented to the screen
	} SwapChainDetails;

	/*
		Indices (locations) of queue families (if they exist at all)
	*/
	typedef struct QueueFamilyIndices
	{
		std::optional<unsigned int> graphics; // location of the graphics family queue
		std::optional<unsigned int> presentation; // location of the presentation family queue

		bool IsValid()
		{
			return graphics.has_value() && presentation.has_value();
		}

		QueueFamilyIndices() {}
		QueueFamilyIndices(const QueueFamilyIndices& other)
		{
			if (other.graphics.has_value())
			{
				graphics = other.graphics.value();
			}
			if (other.presentation.has_value())
			{
				presentation = other.presentation.value();
			}
		}

		QueueFamilyIndices& operator=(const QueueFamilyIndices& other)
		{
			if (other.graphics.has_value())
			{
				graphics = other.graphics.value();
			}
			if (other.presentation.has_value())
			{
				presentation = other.presentation.value();
			}
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& stream, const QueueFamilyIndices& index)
		{
			stream << std::boolalpha << "graphics: " << (index.graphics.has_value() ? index.graphics.value() : false) << "\npresentation: " << (index.presentation.has_value() ? index.presentation.value() : false);
			return stream;
		}
	} QueueFamilyIndices;

	typedef struct PhysicalDevice
	{
		VkPhysicalDevice device;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		//VkPhysicalDeviceMemoryProperties memoryProperties;
		QueueFamilyIndices queueFamilies;
		SwapChainDetails swapChainDetails;

		PhysicalDevice() {}
		PhysicalDevice(const VkPhysicalDevice& device) : device(device) {}
		PhysicalDevice& operator=(const PhysicalDevice& other)
		{
			device = other.device;
			properties = other.properties;
			features = other.features;
			queueFamilies = other.queueFamilies;
			swapChainDetails = other.swapChainDetails;
			return *this;
		}
	} PhysicalDevice;

	typedef struct LogicalDevice
	{
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentationQueue;
	} LogicalDevice;

	class Device
	{
	public:

		static Device* InitializeDevice(Instance* const instance, Surface* const surface);
		~Device();

	private:

		Device(Instance* instance, Surface* surface);

		void SetupPhysicalDevice();
		void SetupLogicalDevice();

		bool IsDeviceSuitable(PhysicalDevice& device);

		void Device::QueryDeviceQueueFamilies(PhysicalDevice& device);
		void UpdateSwapChainDetails(PhysicalDevice& device);

	private:

		PhysicalDevice mPhysicalDevice;
		LogicalDevice mLogicalDevice;

		Surface* mSurface;
		Instance* mInstance;
	};
}
