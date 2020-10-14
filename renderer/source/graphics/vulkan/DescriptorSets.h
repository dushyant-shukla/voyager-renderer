#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	class DescriptorSets
	{
	public:

		DescriptorSets();
		~DescriptorSets();

		void Setup(VkDescriptorSetLayout layout, VkDescriptorPool pool, unsigned int count);

		const VkDescriptorSet& operator[](unsigned int index);

	public:

		std::vector<VkDescriptorSet> mSets;
	};
}
