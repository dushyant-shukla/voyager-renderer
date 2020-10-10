#include "RendererState.h"

namespace vr
{
	VkDevice RendererState::sLogicalDevice = VK_NULL_HANDLE;
	VkPhysicalDevice RendererState::sPhysicalDevice = VK_NULL_HANDLE;
	VkAllocationCallbacks* RendererState::sAllocationCallbacks = VK_NULL_HANDLE;

	VkQueue RendererState::sGraphicsQueue = VK_NULL_HANDLE;
	VkQueue RendererState::sTransferQueue = VK_NULL_HANDLE;
	VkQueue RendererState::sComputeQueue = VK_NULL_HANDLE;

	VkCommandPool RendererState::sGraphicsCommandPool = VK_NULL_HANDLE;
	VkCommandPool RendererState::sTransferCommandPool = VK_NULL_HANDLE;
}