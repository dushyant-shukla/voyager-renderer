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

		PipelineLayout();
		~PipelineLayout();

		void Cleanup();

		PipelineLayout& Create(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks);

		PipelineLayout& AddDescriptorSetLayout(const DescriptorSetLayout& layout);

		PipelineLayout& AddPushConstant(const VkShaderStageFlags flags, unsigned int offset, unsigned int size);

		const VkPipelineLayout& GetVulkanPipelineLayout();

		void Configure();

	private:

		std::vector<VkDescriptorSetLayout> GetVkDescriptorSets();

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::optional<VkPushConstantRange > mPushConstant;
		std::vector<DescriptorSetLayout> mDescriptorSetLayouts;

		VkPipelineLayout mLayout = VK_NULL_HANDLE;
	};
}
