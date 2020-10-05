#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include "DescriptorSetLayout.h"

namespace vr
{
	class PipelineLayout
	{
	public:

		PipelineLayout(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks);
		~PipelineLayout();

		void AddDescriptorSetLayout(const DescriptorSetLayout& layout);

		void AddPushConstant(const VkShaderStageFlags flags, unsigned int offset, unsigned int size);

		void Create();

	private:

		std::vector<VkDescriptorSetLayout> GetVkDescriptorSets();

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::optional<VkPushConstantRange > mPushConstant;
		std::vector<DescriptorSetLayout> mDescriptorSetLayouts;

		VkPipelineLayout mLayout;
	};
}
