#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	class DescriptorSetLayout
	{
	public:

		DescriptorSetLayout(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks);
		~DescriptorSetLayout();

		void AddLayoutBinding(unsigned int binding,
			VkDescriptorType descriptorType,
			unsigned int descriptorCount,
			VkShaderStageFlags stageFlags,
			const VkSampler* pImmutableSamplers);

		void Create(VkDescriptorSetLayoutCreateFlags flags, void* next);

		const VkDescriptorSetLayout& GetVkDescriptorSetLayout() const;

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;

		VkDescriptorSetLayout mLayout;
	};
}