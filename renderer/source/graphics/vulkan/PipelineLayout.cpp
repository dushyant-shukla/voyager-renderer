#include "PipelineLayout.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	PipelineLayout::PipelineLayout(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks)
		: mLogicalDevice(logicalDevice), mAllocationCallbacks(allocationCallbacks)
	{
	}

	PipelineLayout::~PipelineLayout()
	{
		vkDestroyPipelineLayout(mLogicalDevice, mLayout, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: GRAPHICS PIPELINE LAYOUT");
	}

	void PipelineLayout::AddDescriptorSetLayout(const DescriptorSetLayout& layout)
	{
		mDescriptorSetLayouts.push_back(layout);
	}

	// TODO: Test this
	void PipelineLayout::AddPushConstant(const VkShaderStageFlags flags, unsigned int offset, unsigned int size)
	{
		mPushConstant->stageFlags = flags;
		mPushConstant->offset = offset;
		mPushConstant->size = size;

		// mPushConstant.has_value() check
	}

	void PipelineLayout::Create()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<unsigned int> (mDescriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = mDescriptorSetLayouts.empty() ? nullptr : GetVkDescriptorSets().data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = mPushConstant.has_value() ? &mPushConstant.value() : nullptr;

		CHECK_RESULT(vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutCreateInfo, mAllocationCallbacks, &mLayout), "RESOURCE CREATION FAILED: PIPELINE LAYOUT");
		RENDERER_DEBUG("RESOURCE CREATED: PIPELINE LAYOUT");
	}

	std::vector<VkDescriptorSetLayout> PipelineLayout::GetVkDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts;
		for (auto& layout : mDescriptorSetLayouts)
		{
			layouts.push_back(layout.GetVkDescriptorSetLayout());
		}
		return layouts;
	}
}