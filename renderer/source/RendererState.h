#pragma once

#include <vulkan/vulkan.h>

#define LOGICAL_DEVICE			vr::RendererState::GetLogicalDevice()
#define PHYSICAL_DEVICE			vr::RendererState::GetPhysicalDevice()
#define ALLOCATION_CALLBACK		vr::RendererState::GetAllocationCallbacks()
#define GRAPHICS_QUEUE			vr::RendererState::GetGraphicsQueue()
#define GRAPHICS_CMD_POOL		vr::RendererState::GetGraphicsCommandPool()
#define TRANSFER_QUEUE			vr::RendererState::GetTransferQueue()
#define TRANSFER_CMD_POOL		vr::RendererState::GetTransferCommandPool()
#define COMPUTE_QUEUE			vr::RendererState::GetComputeQueue()

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

		static void SetGraphicsQueue(const VkQueue& graphicsQueue)
		{
			sGraphicsQueue = graphicsQueue;
		}
		static const VkQueue& GetGraphicsQueue()
		{
			return sGraphicsQueue;
		}

		static void SetGraphicsCommandPool(const VkCommandPool& graphicsCommandPool)
		{
			sGraphicsCommandPool = graphicsCommandPool;
		}
		static const VkCommandPool& GetGraphicsCommandPool()
		{
			return sGraphicsCommandPool;
		}

		static void SetTransferQueue(const VkQueue& transferQueue)
		{
			sTransferQueue = transferQueue;
		}
		static const VkQueue& GetTransferQueue()
		{
			return sTransferQueue;
		}

		static void SetTransferCommandPool(const VkCommandPool& transferCommandPool)
		{
			sTransferCommandPool = transferCommandPool;
		}
		static const VkCommandPool& GetTransferCommandPool()
		{
			return sTransferCommandPool;
		}

		static void SetComputeQueue(const VkQueue& computeQueue)
		{
			sComputeQueue = computeQueue;
		}
		static const VkQueue& GetComputeQueue()
		{
			return sComputeQueue;
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
