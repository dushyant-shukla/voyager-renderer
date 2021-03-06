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

		void Create(const int& queueFamilyIndex, const int& count);
		VkCommandBuffer& operator[](const unsigned int& index);
		size_t Size();

	private:

		void CreateCommandPool(const int& queueFamilyIndex);
		void AllocateCommandbuffers(const int& count);

	public:

		VkCommandPool mCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> mCommandbuffers;
	};
}
