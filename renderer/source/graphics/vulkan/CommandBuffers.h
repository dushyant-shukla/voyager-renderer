#pragma once

#include <vulkan/vulkan.h>
#include <vector>

/*
	Could be graphics command buffer
	Could be transfer command buffer etc.
*/

namespace vr
{
	class CommandBuffers
	{
	public:

		CommandBuffers();
		~CommandBuffers();

		void Create(const VkDevice& logicalDevice, VkAllocationCallbacks* allocationCallbacks, const int& queueFamilyIndex, const int& count);
		VkCommandBuffer& operator[](const unsigned int& index);
		size_t Size();

	private:

		void CreateCommandPool(const int& queueFamilyIndex);
		void AllocateCommandbuffers(const int& count);

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		VkCommandPool mCommandPool;
		std::vector<VkCommandBuffer> mCommandbuffers;
	};
}
