#pragma once

#include <vulkan/vulkan.h>

#define LOGICAL_DEVICE			vr::RendererState::GetLogicalDevice()
#define PHYSICAL_DEVICE			vr::RendererState::GetPhysicalDevice()
#define ALLOCATION_CALLBACK		vr::RendererState::GetAllocationCallbacks()

namespace vr
{
	class RendererState
	{
	public:

		static const VkDevice& GetLogicalDevice()
		{
			return sLogicalDevice;
		}

		static const VkPhysicalDevice& GetPhysicalDevice()
		{
			return sPhysicalDevice;
		}

		static VkAllocationCallbacks* GetAllocationCallbacks()
		{
			return sAllocationCallbacks;
		}

		static void SetLogicalDevice(const VkDevice& device)
		{
			sLogicalDevice = device;
		}

		static void SetPhysicalDevice(const VkPhysicalDevice& physicalDevice)
		{
			sPhysicalDevice = physicalDevice;
		}

		static void SetAllocationCallbacks(VkAllocationCallbacks* allocationCallbacks)
		{
			sAllocationCallbacks = allocationCallbacks;
		}

	private:

		// devices
		static VkDevice sLogicalDevice;
		static VkPhysicalDevice sPhysicalDevice;
		static VkAllocationCallbacks* sAllocationCallbacks;

		// queues
		static VkQueue sGraphicsQueue;
		static VkQueue sTransferQueue;
		static VkQueue sComputeQueue;

		// command pools
		static VkCommandPool sGraphicsCommandPool;
		static VkCommandPool sTransferCommandPool;
	};
}
