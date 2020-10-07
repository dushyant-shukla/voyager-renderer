#include "PipelineLayout.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	PipelineLayout::PipelineLayout()
	{
	}

	PipelineLayout::~PipelineLayout()
	{
	}

	void PipelineLayout::Cleanup()
	{
		vkDestroyPipelineLayout(mLogicalDevice, mLayout, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: GRAPHICS PIPELINE LAYOUT");
	}

	PipelineLayout& PipelineLayout::Create(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks)
	{
		mLogicalDevice = logicalDevice;
		mAllocationCallbacks = allocationCallbacks;
		return *this;
	}

	PipelineLayout& PipelineLayout::AddDescriptorSetLayout(const DescriptorSetLayout& layout)
	{
		mDescriptorSetLayouts.push_back(layout);
		return *this;
	}

	// TODO: Test this
	PipelineLayout& PipelineLayout::AddPushConstant(const VkShaderStageFlags flags, unsigned int offset, unsigned int size)
	{
		mPushConstant->stageFlags = flags;
		mPushConstant->offset = offset;
		mPushConstant->size = size;

		// mPushConstant.has_value() check
		return *this;
	}

	void PipelineLayout::Configure()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<unsigned int> (mDescriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = mDescriptorSetLayouts.empty() ? nullptr : GetVkDescriptorSets().data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = mPushConstant.has_value() ? 1 : 0;
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