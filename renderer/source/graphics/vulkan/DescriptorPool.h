#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	class DescriptorPool
	{
	public:

		DescriptorPool();
		~DescriptorPool();

		void Create(VkDescriptorPoolCreateFlags flags, unsigned int maxSets, void* next);

		DescriptorPool& AddPoolSize(const VkDescriptorType type, unsigned int count);

		const VkDescriptorPool& GetVulkanDescriptorPool();

	private:

		VkDescriptorPool mPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorPoolSize> mPoolSizes;
	};
}
