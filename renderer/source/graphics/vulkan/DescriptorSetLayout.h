#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	class DescriptorSetLayout
	{
	public:

		DescriptorSetLayout();
		~DescriptorSetLayout();

		DescriptorSetLayout& AddLayoutBinding(unsigned int binding,
			VkDescriptorType descriptorType,
			unsigned int descriptorCount,
			VkShaderStageFlags stageFlags,
			const VkSampler* pImmutableSamplers);

		void Create(VkDescriptorSetLayoutCreateFlags flags, void* next);

		const VkDescriptorSetLayout& GetVkDescriptorSetLayout();

	public:

		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;

		VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;
	};
}