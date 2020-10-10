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

		PipelineLayout& AddDescriptorSetLayout(const VkDescriptorSetLayout& layout);

		PipelineLayout& AddPushConstant(const VkShaderStageFlags flags, unsigned int offset, unsigned int size);

		const VkPipelineLayout& GetVulkanPipelineLayout();

		void Configure();

	private:

		std::optional<VkPushConstantRange > mPushConstant;
		std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;

		VkPipelineLayout mLayout = VK_NULL_HANDLE;
	};
}
